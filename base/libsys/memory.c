#include <proto/exec.h>

#include "common.h"
#include "config.h"
#include "memory.h"

#define KB(x) (((x) + 1023) >> 10)

int __chipmem, __fastmem; /* Configuration variables placed in common area. */

#define BLK_MAGIC 0xDEADC0DE
#define BLK_UNIT  8
#define BLK_ALIGN __attribute__((aligned(BLK_UNIT)))

typedef struct Block BlockT;

struct Block {
  u_int magic;
  int size;         /* including header; size < 0 => used, size > 0 => free */
  union {
    struct {
      BlockT *_prev;
      BlockT *_next;
    } list;
    u_char _data[0];
  } u;
} BLK_ALIGN;

#define prev u.list._prev
#define next u.list._next
#define data u._data

typedef struct Area AreaT;

struct Area {
  AreaT  *succ;      /* link to next area */
  u_int  attributes; /* memory attributes */
  BlockT *first;     /* first free block */
  BlockT *last;      /* last free block */
  void   *upper;     /* upper memory bound + 1 */
  u_int  freeMem;    /* total number of free bytes */
  u_int  maxUsage;   /* minimum recorded number of free bytes */
  BlockT lower[0];   /* first block */
};

#define INSIDE(ptr, area) \
  ((void *)(ptr) >= (void *)((area)->lower) && (void *)(ptr) <= (void *)((area)->upper))
#define OUTSIDE(ptr, area) \
  ((void *)(ptr) < (void *)((area)->lower) || (void *)(ptr) > (void *)((area)->upper))

static AreaT *chip, *fast;

static const char *MemoryName(u_int attributes) {
  if (attributes & MEMF_CHIP)
    return "chip";
  if (attributes & MEMF_FAST)
    return "fast";
  return "public";
}

__regargs static AreaT *MemPoolAlloc(u_int byteSize, u_int attributes) {
  AreaT *area; 

  byteSize = align(byteSize, BLK_UNIT);

tryagain:
  if (!(area = AllocMem(byteSize + sizeof(AreaT), attributes))) {
    /* Allocate from public memory if there's not enough fast memory. */
    if (attributes & MEMF_FAST) {
      attributes |= MEMF_PUBLIC;
      attributes &= ~MEMF_FAST;
      goto tryagain;
    }
  
    Panic("[Mem] Failed to allocate %s memory pool of %dkB!\n",
          MemoryName(attributes), KB(byteSize));
  } 

  area->succ = NULL;
  area->attributes = attributes | MEMF_PUBLIC;
  area->first = area->lower;
  area->last = area->lower;
  area->upper = (void *)area->lower + byteSize;
  area->freeMem = byteSize;
  area->maxUsage = 0;

  /* Set up first chunk in the freelist */
  area->lower->magic = BLK_MAGIC;
  area->lower->size = byteSize;
  area->lower->prev = NULL;
  area->lower->next = NULL;

  return area;
}

void MemInit(void) {
  if (__chipmem == 0)
    __chipmem = DEFAULT_CHIP_CHUNK;
  chip = MemPoolAlloc(__chipmem, MEMF_CHIP);

  if (__fastmem == 0)
    __fastmem = DEFAULT_FAST_CHUNK;
  fast = MemPoolAlloc(__fastmem, MEMF_FAST);

  fast->succ = chip;
}

void MemKill(void) {
  if (chip && fast)
    Log("[Mem] Max usage - CHIP: %dkB FAST: %dkB\n",
        KB(chip->maxUsage), KB(fast->maxUsage));

  if (chip)
    FreeMem(chip, __chipmem + sizeof(AreaT));
  if (fast)
    FreeMem(fast, __fastmem + sizeof(AreaT));
}

ADD2INIT(MemInit, -20);
ADD2EXIT(MemKill, -20);

static inline BlockT *BlockAfter(BlockT *blk) {
  return (void *)blk + abs(blk->size);
}

static __regargs AreaT *FindAreaOf(void *memoryBlock) {
  AreaT *area = fast;

  while (area && OUTSIDE(memoryBlock, area))
    area = area->succ;

  return area;
}

static __regargs bool CheckBlockInternal(BlockT *blk, AreaT *area) {
  if (!blk)
    return true;
  /* block marker damaged ? */
  if (blk->magic != BLK_MAGIC)
    return false;
  /* block size damaged ? */
  if (OUTSIDE(BlockAfter(blk), area))
    return false;
  if (blk->size < 0)
    return true;
  /* backward pointer damaged ? */
  if (blk->prev && OUTSIDE(blk->prev, area))
    return false;
  /* forward pointer damaged ? */
  if (blk->next && OUTSIDE(blk->next, area))
    return false;
  return true;
}

static __regargs void MemDebugInternal(AreaT *area, const char *msg) {
  BlockT *blk = (BlockT *)area->lower;

  Log("%s Area $%p - $%p of %s memory (%dkB free)\n", msg, area->lower,
      area->upper, MemoryName(area->attributes), KB(area->freeMem));

  do {
    if (blk->size < 0) {
      Log(" $%p U %6d\n", blk, -blk->size);
    } else {
      Log(" $%p F %6d ($%p, $%p)\n", blk, blk->size, blk->prev, blk->next);
    }

    if (!CheckBlockInternal(blk, area))
      Panic("%s Block at $%p damaged!\n", msg, blk);

    blk = BlockAfter(blk);
  } while ((void *)blk < area->upper);
}

__regargs void MemDebug(u_int attributes) {
  AreaT *area = fast;
  
  while (area) {
    if (area->attributes & attributes)
      MemDebugInternal(area, "[MemDebug]");
    area = area->succ;
  }
}

static __regargs void CheckBlock(BlockT *blk, AreaT *area, const char *msg) {
  if (!CheckBlockInternal(blk, area))
    MemDebugInternal(area, msg);
}

static __regargs int MemLargest(AreaT *area) {
  BlockT *curr = area->first;
  int size = 0;

  while (curr) {
    CheckBlock(curr, area, "[MemLargest]");
    if (curr->size > size)
      size = curr->size;
    curr = curr->next;
  }

  return size;
}

__regargs int MemAvail(u_int attributes) {
  if (attributes & MEMF_LARGEST) {
    if (attributes & MEMF_CHIP)
      return MemLargest(chip);
    else if (attributes & MEMF_FAST)
      return MemLargest(fast);
    else {
      int chipMax = MemLargest(chip);
      int fastMax = MemLargest(fast);
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

__regargs int MemUsed(u_int attributes) {
  int chipUsed = __chipmem - chip->freeMem;
  int fastUsed = __fastmem - fast->freeMem;

  if (attributes & MEMF_CHIP)
    return chipUsed;
  if (attributes & MEMF_FAST)
    return fastUsed;
  return chipUsed + fastUsed;
}

static BlockT *FindBlockForward(AreaT *area, int size) {
  BlockT *blk = area->first;

  while (blk) {
    CheckBlock(blk, area, "[MemAlloc]");
    if (blk->size >= size)
      break;
    blk = blk->next;
  }

  return blk;
}

static BlockT *FindBlockReverse(AreaT *area, int size) {
  BlockT *blk = area->last;

  while (blk) {
    CheckBlock(blk, area, "[MemAlloc]");
    if (blk->size >= size)
      break;
    blk = blk->prev;
  }

  return blk;
}

static void SplitBlockForward(BlockT *blk, AreaT *area, int size) {
  BlockT *new = (BlockT *)((void *)blk + size);

  new->magic = BLK_MAGIC;
  new->size = blk->size - size;
  new->prev = blk;
  new->next = blk->next;

  if (new->next) new->next->prev = new; else area->last = new;

  blk->size = size;
  blk->next = new;
}

static BlockT *SplitBlockReverse(BlockT *blk, AreaT *area, int size) {
  BlockT *new = (BlockT *)((void *)blk + (blk->size - size));

  new->magic = BLK_MAGIC;
  new->size = size;
  new->prev = blk;
  new->next = blk->next;

  if (new->next) new->next->prev = new; else area->last = new;

  blk->size = blk->size - size;
  blk->next = new;

  return new;
}

static void AllocBlock(BlockT *blk, AreaT *area) {
  area->freeMem -= blk->size;

  /* Take the block out of the free list. */
  if (blk->prev) blk->prev->next = blk->next; else area->first = blk->next;
  if (blk->next) blk->next->prev = blk->prev; else area->last = blk->prev;
  blk->prev = NULL;
  blk->next = NULL;
  blk->size = -blk->size;

  {
    BlockT *last = area->last;

    if (last && ((void *)last + last->size == area->upper)) {
      u_int maxUsage = (void *)last - (void *)area->lower + BLK_UNIT;
      if (area->maxUsage < maxUsage)
        area->maxUsage = maxUsage;
    } else {
      area->maxUsage = (void *)area->upper - (void *)area->lower;
    }
  }
}

__regargs void *MemAlloc(u_int byteSize, u_int attributes) {
  u_int size = align(offsetof(BlockT, data) + byteSize, BLK_UNIT);
  AreaT *area = fast;
  BlockT *blk = NULL;

  while (area) {
    if (area->attributes & attributes) {
      if (attributes & MEMF_REVERSE)
        blk = FindBlockReverse(area, size);
      else
        blk = FindBlockForward(area, size);
      if (blk)
        break;
    }
    area = area->succ;
  }

  if (!blk) {
    Log("[MemAlloc] Failed to allocate %dB of %s memory.\n",
        byteSize, MemoryName(attributes));
    MemDebug(attributes);
    exit(10);
  }

  /* Split the block if can. */
  if (blk->size >= (int)(size + sizeof(BlockT))) {
    if (attributes & MEMF_REVERSE)
      blk = SplitBlockReverse(blk, area, size);
    else
      SplitBlockForward(blk, area, size);
  }

  AllocBlock(blk, area);

  if (attributes & MEMF_CLEAR)
    memset(blk->data, 0, byteSize);

  return blk->data;
}

static void FreeBlock(BlockT *blk, AreaT *area) {
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
      CheckBlock(pred->next, area, "[MemFree]");
      if (blk > pred && blk < pred->next)
        break;
      pred = pred->next;
    }

    blk->next = pred->next;
    blk->prev = pred;
    pred->next->prev = blk;
    pred->next = blk;
  }

  blk->size = -blk->size;
  area->freeMem += blk->size;
}

static void MergeBlock(BlockT *blk, AreaT *area) {
  /* Check if can merge with preceding block. */
  if (blk->prev && (BlockAfter(blk->prev) == blk)) {
    BlockT *pred = blk->prev;

    pred->size += blk->size;
    pred->next = blk->next;
    if (blk->next) pred->next->prev = pred; else area->last = pred;
    blk = pred;
  }

  /* Check if can merge with succeeding block. */
  if (BlockAfter(blk) == blk->next) {
    BlockT *succ = blk->next;

    blk->size += succ->size;
    blk->next = succ->next;
    if (succ->next) blk->next->prev = blk; else area->last = blk;
  }
}

__regargs void MemFree(void *memoryBlock) {
  if (memoryBlock) {
    AreaT *area = FindAreaOf(memoryBlock);

    if (area) {
      BlockT *blk = (BlockT *)(memoryBlock - offsetof(BlockT, data));

      CheckBlock(blk, area, "[MemFree]");
      CheckBlock(area->first, area, "[MemFree]");
      CheckBlock(area->last, area, "[MemFree]");

      FreeBlock(blk, area);
      MergeBlock(blk, area);
    } else {
      Panic("[MemFree] block $%p not found!\n", memoryBlock);
    }
  }
}

__regargs void MemResize(void *memoryBlock, u_int byteSize) {
  AreaT *area = FindAreaOf(memoryBlock);

  if (area) {
    BlockT *blk = (BlockT *)(memoryBlock - offsetof(BlockT, data));
    int blkSize = -blk->size;
    int size = align(offsetof(BlockT, data) + byteSize, BLK_UNIT);

    CheckBlock(blk, area, "[MemResize]");
    CheckBlock(area->first, area, "[MemResize]");
    CheckBlock(area->last, area, "[MemResize]");

    if (size < blkSize) {
      u_int leftover = blkSize - size;

      /* shrink block if unused memory at the end of block is large enough */
      if (leftover >= sizeof(BlockT)) {
        BlockT *new = (void *)blk + size;

        blk->size = -size;
        new->magic = BLK_MAGIC;
        new->size = -leftover;

        FreeBlock(new, area);
        MergeBlock(new, area);
      }
    } else if (size > blkSize) {
      BlockT *succ = BlockAfter(blk);

      CheckBlock(succ, area, "[MemResize]");

      if (succ->size >= size - blkSize) {
        /* expand block if enough free space in successor block */
        SplitBlockForward(succ, area, size - blkSize);
        AllocBlock(succ, area);

        blk->size = -size;
      } else {
        /* the block would've had to be moved to satisfy the request */
        Log("[MemResize] Failed to resize $%p to %dB.\n", blk, byteSize);
        MemDebugInternal(area, "[MemResize]");
        exit(10);
      }
    }
  } else {
    Panic("[MemResize] block $%p not found!\n", memoryBlock);
  }
}

__regargs int MemTypeOf(void *address) {
  if (INSIDE(address, chip))
    return MEMF_CHIP;
  if (INSIDE(address, fast))
    return MEMF_FAST;
  return -1;
}
