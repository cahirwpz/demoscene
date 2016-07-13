#include <proto/exec.h>

#include "common.h"
#include "config.h"
#include "memory.h"
#include "io.h"

#define KB(x) (((x) + 1023) >> 10)

LONG __chipmem, __fastmem; /* Configuration variables placed in common area. */

#define BLK_MAGIC 0xDEADC0DE
#define BLK_USED  1
#define BLK_UNIT  8
#define BLK_MASK  (BLK_UNIT - 1)
#define BLK_ALIGN __attribute__((aligned(BLK_UNIT)))

typedef struct Block {
  ULONG magic;
  ULONG size;
  struct Block *prev;
  struct Block *next;
} BLK_ALIGN BlockT;

typedef struct Area {
  char   *name;    /* description */
  BlockT *first;   /* first free region */
  BlockT *last;    /* last free region */
  APTR   upper;    /* upper memory bound + 1 */
  ULONG  freeMem;  /* total number of free bytes */
  ULONG  maxUsage; /* minimum recorded number of free bytes */
  BlockT lower[0]; /* first block */
} AreaT;

#define INSIDE(ptr, area) \
  ((APTR)(ptr) >= (APTR)((area)->lower) && (APTR)(ptr) <= (APTR)((area)->upper))
#define OUTSIDE(ptr, area) \
  ((APTR)(ptr) < (APTR)((area)->lower) || (APTR)(ptr) > (APTR)((area)->upper))

static AreaT *chip, *fast;

__regargs static AreaT *MemPoolAlloc(ULONG byteSize, ULONG attributes) {
  char *const name = (attributes & MEMF_CHIP) ? "chip" : "fast";
  AreaT *area; 

  byteSize = (byteSize + BLK_MASK) & ~BLK_MASK;
  
  if (!(area = AllocMem(byteSize + sizeof(AreaT), attributes)))
    Panic("[Mem] Failed to allocate %s memory pool of %ldkB!\n",
          name, KB(byteSize));

  area->name = name;
  area->first = area->lower;
  area->last = area->lower;
  area->upper = (APTR)area->lower + byteSize;
  area->freeMem = byteSize;
  area->maxUsage = 0;

  /* Set up first chunk in the freelist */
  area->lower->magic = BLK_MAGIC;
  area->lower->size = byteSize;
  area->lower->prev = NULL;
  area->lower->next = NULL;

  return area;
}

void MemInit() {
  if (__chipmem == 0)
    __chipmem = DEFAULT_CHIP_CHUNK;
  chip = MemPoolAlloc(__chipmem, MEMF_CHIP);

  if (__fastmem == 0)
    __fastmem = DEFAULT_FAST_CHUNK;
  fast = MemPoolAlloc(__fastmem, MEMF_PUBLIC);
}

void MemKill() {
  if (chip && fast)
    Log("[Mem] Max usage - CHIP: %ldkB FAST: %ldkB\n",
        KB(chip->maxUsage), KB(fast->maxUsage));

  if (chip)
    FreeMem(chip, __chipmem + sizeof(AreaT));
  if (fast)
    FreeMem(fast, __fastmem + sizeof(AreaT));
}

ADD2INIT(MemInit, -20);
ADD2EXIT(MemKill, -20);

static __regargs BOOL BlockCheckInternal(BlockT *blk, AreaT *area) {
  if (!blk)
    return TRUE;
  /* block marker damaged ? */
  if (blk->magic != BLK_MAGIC)
    return FALSE;
  /* block size damaged ? */
  if (OUTSIDE((APTR)blk + (blk->size & ~BLK_MASK), area))
    return FALSE;
  if (blk->size & BLK_USED)
    return TRUE;
  /* backward pointer damaged ? */
  if (blk->prev && OUTSIDE(blk->prev, area))
    return FALSE;
  /* forward pointer damaged ? */
  if (blk->next && OUTSIDE(blk->next, area))
    return FALSE;
  return TRUE;
}

static __regargs void MemDebugInternal(AreaT *area, const char *msg) {
  BlockT *blk = (BlockT *)area->lower;

  Log("%s Area $%lx - $%lx of %s memory (%ldkB free):\n", msg,
      (LONG)area->lower, (LONG)area->upper, area->name, KB(area->freeMem));

  do {
    if (blk->size & BLK_USED) {
      Log(" $%lx U %6ld\n", (LONG)blk, blk->size & ~BLK_MASK);
    } else {
      Log(" $%lx F %6ld ($%lx, $%lx)\n",
          (LONG)blk, blk->size & ~BLK_MASK, (LONG)blk->prev, (LONG)blk->next);
    }

    if (!BlockCheckInternal(blk, area))
      Panic("%s Block at $%lx damaged!\n", msg, (LONG)blk);

    blk = (BlockT *)((APTR)blk + (blk->size & ~BLK_MASK));
  } while ((APTR)blk < area->upper);
}

__regargs void MemDebug(ULONG attributes) {
  if (attributes & (MEMF_CHIP | MEMF_PUBLIC))
    MemDebugInternal(chip, "[MemDebug]");
  if (attributes & (MEMF_FAST | MEMF_PUBLIC))
    MemDebugInternal(fast, "[MemDebug]");
}

static __regargs void BlockCheck(BlockT *blk, AreaT *area, const char *msg) {
  if (!BlockCheckInternal(blk, area))
    MemDebugInternal(area, msg);
}

static __regargs LONG MemLargest(AreaT *area) {
  BlockT *curr = area->first;
  LONG size = 0;

  while (curr) {
    BlockCheck(curr, area, "[MemLargest]");
    if (curr->size > size)
      size = curr->size;
    curr = curr->next;
  }

  return size;
}

__regargs LONG MemAvail(ULONG attributes) {
  if (attributes & MEMF_LARGEST) {
    if (attributes & MEMF_CHIP)
      return MemLargest(chip);
    else if (attributes & MEMF_FAST)
      return MemLargest(fast);
    else {
      LONG chipMax = MemLargest(chip);
      LONG fastMax = MemLargest(fast);
      return max(chipMax, fastMax);
    }
  } else {
    if (attributes & MEMF_CHIP)
      return chip->freeMem;
    if (attributes & MEMF_FAST)
      return fast->freeMem;
    return chip->freeMem + fast->freeMem;
  }
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

static BlockT *MemAllocInternal(AreaT *area, ULONG byteSize, ULONG attributes)
{
  BlockT *curr;
  ULONG size = (byteSize + BLK_UNIT * 2 - 1) & ~BLK_MASK;

  BlockCheck(area->first, area, "[MemAlloc]");
  BlockCheck(area->last, area, "[MemAlloc]");

  /* First fit. */
  if (attributes & MEMF_REVERSE) {
    curr = area->last;

    while (curr) {
      BlockCheck(curr->prev, area, "[MemAlloc]");
      if (curr->size >= size)
        break;
      curr = curr->prev;
    }
  } else {
    curr = area->first;

    while (curr) {
      BlockCheck(curr->next, area, "[MemAlloc]");
      if (curr->size >= size)
        break;
      curr = curr->next;
    }
  }

  if (!curr)
    return NULL;

  /* Split the block if can. */
  if (curr->size >= size + BLK_UNIT * 2) {
    if (attributes & MEMF_REVERSE) {
      BlockT *new = (BlockT *)((APTR)curr + (curr->size - size));

      new->magic = BLK_MAGIC;
      new->size = size;
      new->prev = curr;
      new->next = curr->next;
      (new->next ? new->next->prev : area->last) = new;

      curr->size = curr->size - size;
      curr->next = new;
      curr = new;
    } else {
      BlockT *new = (BlockT *)((APTR)curr + size);

      new->magic = BLK_MAGIC;
      new->size = curr->size - size;
      new->prev = curr;
      new->next = curr->next;
      (new->next ? new->next->prev : area->last) = new;

      curr->size = size;
      curr->next = new;
    }
  }

  area->freeMem -= curr->size;

  /* Take the block out of the free list. */
  (curr->prev ? curr->prev->next : area->first) = curr->next;
  (curr->next ? curr->next->prev : area->last) = curr->prev;
  curr->prev = NULL;
  curr->next = NULL;
  curr->size |= BLK_USED;

  {
    BlockT *last = area->last;

    if (last && ((APTR)last + last->size == area->upper)) {
      LONG maxUsage = (APTR)last - (APTR)area->lower + BLK_UNIT;
      if (area->maxUsage < maxUsage)
        area->maxUsage = maxUsage;
    } else {
      area->maxUsage = (APTR)area->upper - (APTR)area->lower;
    }
  }

  return curr;
}

__regargs APTR MemAlloc(ULONG byteSize, ULONG attributes) {
  BlockT *blk = NULL;
  AreaT *area = NULL;
  APTR ptr;

  byteSize = (byteSize + BLK_MASK) & ~BLK_MASK;

  if ((attributes & MEMF_FAST) || (attributes & MEMF_PUBLIC)) {
    area = fast; blk = MemAllocInternal(fast, byteSize, attributes);
  }
  if ((attributes & MEMF_CHIP) || (!blk && (attributes & MEMF_PUBLIC))) {
    area = chip; blk = MemAllocInternal(chip, byteSize, attributes);
  }

  if (!blk) {
    const char *name;

    if (attributes & MEMF_CHIP)
      name = "chip";
    else if (attributes & MEMF_FAST)
      name = "fast";
    else
      name = "public";

    Log("[MemAlloc] Failed to allocate %ld bytes of %s memory.\n",
        byteSize, name);
    MemDebug(attributes);
    exit(10);
  }

  ptr = (APTR)blk + BLK_UNIT;

  if (attributes & MEMF_CLEAR)
    memset(ptr, 0, byteSize);

  return ptr;
}

static void MemFreeInternal(AreaT *area, BlockT *blk) {
  BlockCheck(blk, area, "[MemFree]");
  BlockCheck(area->first, area, "[MemFree]");
  BlockCheck(area->last, area, "[MemFree]");

  blk->size &= ~BLK_USED;

  area->freeMem += blk->size;

  /* Put the block onto free list. */
  if (blk < area->first) {
    blk->prev = NULL;
    blk->next = area->first;
    blk->next->prev = blk;
    area->first = blk;
  } else if (blk > area->last) {
    blk->prev = area->last;
    blk->prev->next = blk;
    blk->next = NULL;
    area->last = blk;
  } else {
    BlockT *pred = area->first;

    while (pred) {
      BlockCheck(pred->next, area, "[MemFree]");
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
    BlockT *blk = (BlockT *)((APTR)memoryBlock - BLK_UNIT);

    if (INSIDE(memoryBlock, chip))
      MemFreeInternal(chip, blk);
    else if (INSIDE(memoryBlock, fast))
      MemFreeInternal(fast, blk);
    else
      Log("[MemFree] block $%lx not found!\n", (LONG)memoryBlock);
  }
}

__regargs LONG MemTypeOf(APTR address) {
  if (INSIDE(address, chip))
    return MEMF_CHIP;
  if (INSIDE(address, fast))
    return MEMF_FAST;
  return -1;
}
