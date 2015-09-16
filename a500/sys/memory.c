#include <proto/exec.h>

#include "common.h"
#include "memory.h"
#include "io.h"

LONG __chipmem, __fastmem; /* Configuration variables placed in common area. */

#define REDZONE 1

typedef struct MemHeader MemHeaderT;
typedef struct MemChunk MemChunkT;

typedef struct {
  ULONG size;
  UBYTE data[0];
} MemBlockT;

static MemHeaderT *chip, *fast;
static LONG chipMin, fastMin;
static BOOL memReady = FALSE;

__regargs static void MemDebug(MemHeaderT *mem) {
  MemChunkT *mc = mem->mh_First;

  Log("Free %s memory map:\n", mem->mh_Node.ln_Name);
  while (mc) {
    Log("$%08lx : %ld\n", (LONG)mc, (LONG)mc->mc_Bytes);
    mc = mc->mc_Next;
  }
  Log("Total free %s memory: %ld\n", mem->mh_Node.ln_Name, mem->mh_Free);
}

__regargs static MemHeaderT *MemPoolAlloc(ULONG byteSize, ULONG attributes) {
  MemHeaderT *mem = AllocMem(byteSize + sizeof(struct MemHeader), attributes);
  char *const memName = (attributes & MEMF_CHIP) ? "chip" : "public";

  if (mem) {
    /* Set up first chunk in the freelist */
    MemChunkT *mc = (APTR)mem + sizeof(MemHeaderT);
    mc->mc_Next  = NULL;
    mc->mc_Bytes = byteSize;

    mem->mh_Node.ln_Type = NT_MEMORY;
    mem->mh_Node.ln_Name = memName;
    mem->mh_First = mc;
    mem->mh_Lower = (APTR)mc;
    mem->mh_Upper = (APTR)mc + byteSize;
    mem->mh_Free  = byteSize;
    return mem;
  }

  Print("Failed to allocate %ld bytes of %s memory in one chunk!\n",
        byteSize, memName);
  return mem;
}

void MemInit() {
  if (__chipmem == 0)
    __chipmem = 262144;
  if (__fastmem == 0)
    __fastmem = 262144;

  if (!(chip = MemPoolAlloc(__chipmem, MEMF_CHIP)))
    exit();

  if (!(fast = MemPoolAlloc(__fastmem, MEMF_PUBLIC)))
    exit();

  chipMin = __chipmem;
  fastMin = __fastmem;
  memReady = TRUE;
}

void MemKill() {
  if (memReady)
    Log("[Memory] Max usage - CHIP: %ld FAST: %ld\n",
        __chipmem - chipMin, __fastmem - fastMin);
  if (chip)
    FreeMem(chip, __chipmem + sizeof(MemHeaderT));
  if (fast)
    FreeMem(fast, __fastmem + sizeof(MemHeaderT));
}

ADD2INIT(MemInit, -20);
ADD2EXIT(MemKill, -20);

LONG MemAvail(ULONG attributes asm("d1")) {
  return (attributes & MEMF_CHIP) ? chip->mh_Free : fast->mh_Free;
}

LONG MemUsed(ULONG attributes asm("d1")) {
  return (attributes & MEMF_CHIP) ? 
    (__chipmem - chip->mh_Free) : (__fastmem - fast->mh_Free);
}

static __attribute__((noreturn))
  void MemPanic(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) 
{
  MemHeaderT *mem = (attributes & MEMF_CHIP) ? chip : fast;

  Log("Failed to allocate %ld bytes of %s memory.\n",
      byteSize, mem->mh_Node.ln_Name);

  MemDebug(mem);
  MemKill();
  exit();
}

static inline APTR MemAllocInternal(ULONG byteSize, ULONG attributes) {
  APTR ptr;

#if REDZONE
  ptr = Allocate((attributes & MEMF_CHIP) ? chip : fast, byteSize + 4);
  {
    UBYTE *marker = ptr + byteSize;
    *marker++ = 0xDE;
    *marker++ = 0xAD;
    *marker++ = 0xC0;
    *marker++ = 0xDE;
  }
#else
  ptr = Allocate((attributes & MEMF_CHIP) ? chip : fast, byteSize);
#endif


  if (attributes & MEMF_CHIP) {
    if (chip->mh_Free < chipMin) {
      // Log("[Memory] Usage peak - CHIP: %ld\n", __chipmem - chipMin);
      chipMin = chip->mh_Free;
    }
  } else {
    if (fast->mh_Free < fastMin) {
      // Log("[Memory] Usage peak - FAST: %ld\n", __fastmem - fastMin);
      fastMin = fast->mh_Free;
    }
  }

  if (!ptr)
    MemPanic(byteSize, attributes);

  if (attributes & MEMF_CLEAR)
    memset(ptr, 0, byteSize);

  return ptr;
}


APTR MemAlloc(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  MemBlockT *mb = MemAllocInternal(byteSize + sizeof(MemBlockT), attributes);
  mb->size = byteSize;
  return mb->data;
}

#if REDZONE
static BOOL RedZoneValid(APTR memoryBlock asm("a1"), ULONG byteSize asm("d0")) {
  UBYTE *marker = memoryBlock + byteSize;
  UBYTE d0 = *marker++;
  UBYTE d1 = *marker++;
  UBYTE d2 = *marker++;
  UBYTE d3 = *marker++;
  ULONG d = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;

  return d == 0xDEADC0DE;
}
#endif

static inline void MemFreeInternal(APTR memoryBlock, ULONG byteSize) {
  if (memoryBlock) {
    MemHeaderT *mem;

    if (memoryBlock >= chip->mh_Lower && memoryBlock <= chip->mh_Upper) {
      mem = chip;
    } else if (memoryBlock >= fast->mh_Lower && memoryBlock <= fast->mh_Upper) {
      mem = fast;
    } else {
      mem = NULL;
    }

    if (mem) {
#if REDZONE
      if (!RedZoneValid(memoryBlock, byteSize))
        Log("[Bug] MemFree: block $%lx (size: $%lx) marker damaged!\n",
            (LONG)memoryBlock, byteSize);
      byteSize += 4;
#endif
      Deallocate(mem, memoryBlock, byteSize);
    } else {
      Log("[Bug] MemFree: block $%lx (size: $%lx) not found!\n",
          (LONG)memoryBlock, (LONG)byteSize);
    }
  }
}

void MemFree(APTR memoryBlock asm("a1")) {
  if (memoryBlock) {
    MemBlockT *mb = (MemBlockT *)((APTR)memoryBlock - sizeof(MemBlockT));
    MemFreeInternal(mb, mb->size + sizeof(MemBlockT));
  }
}
