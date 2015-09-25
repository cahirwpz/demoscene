#include <proto/exec.h>

#include "common.h"
#include "memory.h"
#include "io.h"

LONG __chipmem, __fastmem; /* Configuration variables placed in common area. */

#define BLK_MAGIC 0xDEADC0DE
#define BLK_USED  1
#define BLK_UNIT  8
#define BLK_MASK  (BLK_UNIT - 1)

typedef struct Block {
  ULONG magic;
  ULONG size;
  struct Block *prev;
  struct Block *next;
} BlockT;

typedef struct Area {
  char   *name;   /* description */
  BlockT *first;  /* first free region */
  BlockT *last;   /* last free region */
  APTR   lower;   /* lower memory bound */
  APTR   upper;   /* upper memory bound + 1 */
  ULONG  freeMem; /* total number of free bytes */
  ULONG  minFree; /* minimum recorded number of free bytes */
} __attribute__((aligned(BLK_UNIT))) AreaT;

static AreaT *chip, *fast;
static BOOL memReady = FALSE;

__regargs static AreaT *MemPoolAlloc(ULONG byteSize, ULONG attributes) {
  char *const name = (attributes & MEMF_CHIP) ? "chip" : "fast";
  AreaT *area; 

  byteSize = (byteSize + MEM_BLOCKMASK) & ~MEM_BLOCKMASK;
  
  if ((area = AllocMem(byteSize + sizeof(AreaT), attributes))) {
    /* Set up first chunk in the freelist */
    BlockT *blk = (APTR)area + sizeof(AreaT);

    blk->magic = BLK_MAGIC;
    blk->size = byteSize;
    blk->prev = NULL;
    blk->next = NULL;

    area->name = name;
    area->first = blk;
    area->last = blk;
    area->lower = (APTR)blk;
    area->upper = (APTR)blk + byteSize;
    area->freeMem = byteSize - BLK_UNIT;
    area->minFree = byteSize - BLK_UNIT;
    return area;
  }

  Print("Failed to allocate %ld bytes of %s memory in one chunk!\n",
        byteSize, name);
  return NULL;
}

void MemInit() {
  if (__chipmem == 0)
    __chipmem = 262144;
  if (__fastmem == 0)
    __fastmem = 262144;

  if ((chip = MemPoolAlloc(__chipmem, MEMF_CHIP))) {
    if ((fast = MemPoolAlloc(__fastmem, MEMF_PUBLIC))) {
      memReady = TRUE;
      return;
    }
    FreeMem(chip, __chipmem + sizeof(AreaT));
    chip = NULL;
  }

  exit();
}

void MemKill() {
  if (memReady) {
    Log("[Memory] Max usage - CHIP: %ld FAST: %ld\n",
        __chipmem - chip->minFree, __fastmem - fast->minFree);
    FreeMem(chip, __chipmem + sizeof(AreaT));
    FreeMem(fast, __fastmem + sizeof(AreaT));
  }
}

ADD2INIT(MemInit, -20);
ADD2EXIT(MemKill, -20);

__regargs LONG MemAvail(ULONG attributes) {
  if (attributes & MEMF_CHIP)
    return chip->freeMem;
  if (attributes & MEMF_FAST)
    return fast->freeMem;
  return chip->freeMem + fast->freeMem;
}

__regargs LONG MemUsed(ULONG attributes) {
  LONG chipUsed = __chipmem - chip->freeMem;
  LONG fastUsed = __fastmem - fast->freeMem;

  if (attributes & MEMF_CHIP)
    return chipUsed;
  if (attributes & MEMF_FAST)
    return fastUsed;
  return chipUsed + fastUsed;
}

static __regargs BOOL BlockValidMagic(BlockT *blk, AreaT *area, const char *msg) {
  if (blk->magic != BLK_MAGIC) {
    Log("%s $%lx : marker ($%08lx) damaged!\n", msg, (LONG)blk, blk->magic);
    return FALSE;
  }
  return TRUE;
}

static __regargs BOOL BlockValidSize(BlockT *blk, AreaT *area, const char *msg) {
  APTR ptr = (APTR)blk + (blk->size & ~BLK_MASK);

  if (ptr < area->lower || ptr > area->upper) {
    Log("%s $%lx : size ($%ld) damaged!\n", msg, (LONG)blk, blk->size);
    return FALSE;
  }
  return TRUE;
}

static __regargs BOOL BlockValidPrev(BlockT *blk, AreaT *area, const char *msg) {
  APTR ptr = blk->prev;

  if (ptr && (ptr < area->lower || ptr > area->upper)) {
    Log("%s $%lx : backward pointer ($%lx) damaged!\n",
        msg, (LONG)blk, (LONG)blk->prev);
    return FALSE;
  }
  return TRUE;
}

static __regargs BOOL BlockValidNext(BlockT *blk, AreaT *area, const char *msg) {
  APTR ptr = blk->next;

  if (ptr && (ptr < area->lower || ptr > area->upper)) {
    Log("%s $%lx : forward pointer ($%lx) damaged!\n",
        msg, (LONG)blk, (LONG)blk->prev);
    return FALSE;
  }
  return TRUE;
}

static __regargs void MemDebug(AreaT *area) {
  BlockT *blk = (BlockT *)area->lower;

  Log("Map for %s memory:\n", area->name);
  do {
    if (!BlockValidMagic(blk, area, "[MemDebug]") || 
        !BlockValidSize(blk, area, "[MemDebug]"))
      break;

    if (blk->size & BLK_USED) {
      Log(" $%lx : used, size : %ld\n", (LONG)blk, blk->size & ~BLK_MASK);
    } else {
      if (!(blk->size & BLK_USED)) {
        if (!BlockValidPrev(blk, area, "[MemDebug]") ||
            !BlockValidNext(blk, area, "[MemDebug]"))
          break;

        Log(" $%lx : free, size : %ld, prev : $%lx, next : $%lx\n",
            (LONG)blk, blk->size & ~BLK_MASK, (LONG)blk->prev, (LONG)blk->next);
      }
    }

    blk = (BlockT *)((APTR)blk + (blk->size & ~BLK_MASK));
  } while (blk < area->last);

  Log("Total free %s memory: %ld bytes.\n", area->name, area->freeMem);
}

static __attribute__((noreturn)) __regargs
  void MemPanic(AreaT *area)
{
  MemDebug(area);
  MemKill();
  exit();
}

#define ALLOC 0
#define FREE 1

static const char *action[] = {
  "[Bug] MemAlloc: block", "[Bug] MemFree: block"
};

static void BlockCheck(LONG num, AreaT *area, BlockT *blk) {
  if (blk) {
    if (!BlockValidMagic(blk, area, action[num]))
      MemPanic(area);

    if (!BlockValidSize(blk, area, action[num]))
      MemPanic(area);

    if (!(blk->size & BLK_USED)) {
      if (!BlockValidPrev(blk, area, action[num]))
        MemPanic(area);

      if (!BlockValidNext(blk, area, action[num]))
        MemPanic(area);
    }
  }
}

static BlockT *MemAllocInternal(AreaT *area, ULONG byteSize) {
  BlockT *curr = area->first;
  ULONG size = byteSize + BLK_UNIT;

  BlockCheck(ALLOC, area, area->first);
  BlockCheck(ALLOC, area, area->last);

  /* First fit. */
  while (curr) {
    BlockCheck(ALLOC, area, curr->next);

    if (curr->size >= size)
      break;

    curr = curr->next;
  }

  if (!curr)
    return NULL;

  /* Split the block if can. */
  if (curr->size >= size + BLK_UNIT * 2) {
    BlockT *new = (BlockT *)((APTR)curr + size);

    new->magic = BLK_MAGIC;
    new->size = curr->size - size;
    new->prev = curr;
    new->next = curr->next;
    (new->next) ? (new->next->prev) : (area->last) = new;

    curr->size = size;
    curr->next = new;
  }

  /* Take the block out of the free list. */
  (curr->prev) ? (curr->prev->next) : (area->first) = curr->next;
  (curr->next) ? (curr->next->prev) : (area->last) = curr->prev;
  curr->prev = NULL;
  curr->next = NULL;
  curr->size |= BLK_USED;

  area->freeMem -= size;

  if (area->freeMem < area->minFree) {
    // Log("[Memory] Usage peak - CHIP: %ld\n", __chipmem - chip->min);
    area->minFree = area->freeMem;
  }

  return curr;
}

__regargs APTR MemAlloc(ULONG byteSize, ULONG attributes) {
  BlockT *blk = NULL;
  AreaT *area = NULL;
  APTR ptr;

  byteSize = (byteSize + BLK_MASK) & ~BLK_MASK;

  if (!blk && ((attributes & MEMF_FAST) || (attributes & MEMF_PUBLIC))) {
    area = fast; blk = MemAllocInternal(fast, byteSize);
  }
  if (!blk && ((attributes & MEMF_CHIP) || (attributes & MEMF_PUBLIC))) {
    area = chip; blk = MemAllocInternal(chip, byteSize);
  }

  if (!blk) {
    const char *name;

    if (attributes & MEMF_CHIP)
      name = "chip";
    else if (attributes & MEMF_FAST)
      name = "fast";
    else
      name = "public";

    Log("Failed to allocate %ld bytes of %s memory.\n", byteSize, name);
    MemPanic(area);
  }

  ptr = (APTR)blk + BLK_UNIT;

  if (attributes & MEMF_CLEAR)
    memset(ptr, 0, byteSize);

  return ptr;
}

static void MemFreeInternal(AreaT *area, BlockT *blk) {
  BlockCheck(FREE, area, blk);
  BlockCheck(FREE, area, area->first);
  BlockCheck(FREE, area, area->last);

  blk->size &= ~BLK_USED;

  /* Put the block onto free list. */
  if (blk < area->first) {
    blk->prev = NULL;
    blk->next = area->first;
    area->first = blk;
  } else if (blk > area->last) {
    blk->prev = area->last;
    blk->next = NULL;
    area->last = blk;
  } else {
    BlockT *pred = area->first;

    while (pred) {
      BlockCheck(FREE, area, pred->next);

      if (blk > pred && blk < pred->next)
        break;

      pred = pred->next;
    }

    blk->next = pred->next;
    blk->prev = pred;

    pred->next->prev = blk;
    pred->next = blk;
  }

  /* Check if can merge with preceding block. */
  if (blk->prev && ((APTR)blk->prev + blk->prev->size == (APTR)blk)) {
    BlockT *pred = blk->prev;

    pred->size += blk->size;
    pred->next = blk->next;
    
    (blk->next) ? (pred->next->prev) : (area->last) = pred;

    blk = pred;
  }

  /* Check if can merge with succeeding block. */
  if (blk->next && ((APTR)blk + blk->size == (APTR)blk->next)) {
    BlockT *succ = blk->next;

    blk->size += succ->size;
    blk->next = succ->next;

    (succ->next) ? (blk->next->prev) : (area->last) = blk;
  }
}

__regargs void MemFree(APTR memoryBlock) {
  if (memoryBlock) {
    AreaT *area = NULL;
    BlockT *blk = (BlockT *)((APTR)memoryBlock - BLK_UNIT);

    if (memoryBlock >= chip->lower && memoryBlock < chip->upper)
      area = chip;
    if (memoryBlock >= fast->lower && memoryBlock < fast->upper)
      area = fast;

    if (area) {
      MemFreeInternal(area, blk);
    } else {
      Log("[Bug] MemFree: block $%lx not found!\n", (LONG)memoryBlock);
    }
  }
}
