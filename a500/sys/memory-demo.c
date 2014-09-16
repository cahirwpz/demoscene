#include <proto/exec.h>

#include "common.h"
#include "memory.h"
#include "print.h"

#define BLOCKSIZE (850 * 1024)

static struct MemHeader mh;
static struct MemChunk *mc = NULL;

BOOL MemInit() {
  mc = AllocMem(BLOCKSIZE, MEMF_CHIP);

  if (!mc) {
    Print("Failed to allocate %ld bytes of chip memory in one chunk!\n",
          (LONG)BLOCKSIZE);
    return FALSE;
  }

  mh.mh_Node.ln_Type = NT_MEMORY;
  mh.mh_Node.ln_Name = "memory for demo";
  mh.mh_First = mc;
  mh.mh_Lower = (APTR)mc;
  mh.mh_Upper = (APTR)mc + BLOCKSIZE;
  mh.mh_Free  = BLOCKSIZE;

  /* Set up first chunk in the freelist */
  mc->mc_Next  = NULL;
  mc->mc_Bytes = BLOCKSIZE;

  return TRUE;
}

void MemKill() {
  FreeMem(mc, BLOCKSIZE);
}

static void MemDebug() {
  struct MemChunk *mc = mh.mh_First;

  Log("Free memory map:\n");
  while (mc) {
    Log("$%08lx : %ld\n", (LONG)mc, (LONG)mc->mc_Bytes);
    mc = mc->mc_Next;
  }
  Log("Total free memory: %ld\n", mh.mh_Free);
}

LONG MemAvail() {
  MemDebug();
  return mh.mh_Free;
}

static __attribute__((noreturn)) void MemPanic(ULONG byteSize) {
  Log("Failed to allocate %ld bytes.\n", byteSize);
  if (byteSize < mh.mh_Free)
    MemDebug();
  MemKill();
  exit();
}

APTR MemAlloc(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  APTR ptr = Allocate(&mh, byteSize);

  if (!ptr)
    MemPanic(byteSize);

  memset(ptr, 0, byteSize);

  return ptr;
}

void MemFree(APTR memoryBlock asm("a1"), ULONG byteSize asm("d0")) {
  if (memoryBlock)
    Deallocate(&mh, memoryBlock, byteSize);
}

typedef struct {
  ULONG size;
  UBYTE data[0];
} MemBlockT;

APTR MemAllocAuto(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  MemBlockT *mb = NULL;

  if ((mb = Allocate(&mh, byteSize + sizeof(MemBlockT)))) {
    if (attributes & MEMF_CLEAR)
      memset(mb, 0, byteSize + sizeof(MemBlockT));
    mb->size = byteSize;
  } else {
    MemPanic(byteSize);
  }

  return mb->data;
}

void MemFreeAuto(APTR memoryBlock asm("a1")) {
  if (memoryBlock) {
    MemBlockT *mb = (MemBlockT *)((ULONG *)memoryBlock - 1);
    Deallocate(&mh, mb, mb->size + sizeof(MemBlockT));
  }
}
