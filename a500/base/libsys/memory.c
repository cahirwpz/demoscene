#include <proto/exec.h>

#include "common.h"
#include "config.h"
#include "memory.h"
#include "io.h"

#define KB(x) (((x) + 1023) >> 10)

LONG __chipmem, __fastmem; /* Configuration variables placed in common area. */

#define BLK_MAGIC 0xDEADC0DE
#define BLK_UNIT  8
#define BLK_ALIGN __attribute__((aligned(BLK_UNIT)))

typedef struct Block BlockT;

struct Block {
  LONG magic;
  LONG size;         /* including header; size < 0 => used, size > 0 => free */
  union {
    struct {
      BlockT *_prev;
      BlockT *_next;
    } list;
    UBYTE _data[0];
  } u;
} BLK_ALIGN;

#define prev u.list._prev
#define next u.list._next
#define data u._data

typedef struct Area AreaT;

struct Area {
  AreaT  *succ;      /* link to next area */
  ULONG  attributes; /* memory attributes */
  BlockT *first;     /* first free block */
  BlockT *last;      /* last free block */
  APTR   upper;      /* upper memory bound + 1 */
  ULONG  freeMem;    /* total number of free bytes */
  ULONG  maxUsage;   /* minimum recorded number of free bytes */
  BlockT lower[0];   /* first block */
};

#define INSIDE(ptr, area) \
  ((APTR)(ptr) >= (APTR)((area)->lower) && (APTR)(ptr) <= (APTR)((area)->upper))
#define OUTSIDE(ptr, area) \
  ((APTR)(ptr) < (APTR)((area)->lower) || (APTR)(ptr) > (APTR)((area)->upper))

static AreaT *chip, *fast;

static const char *MemoryName(ULONG attributes) {
  if (attributes & MEMF_CHIP)
    return "chip";
  if (attributes & MEMF_FAST)
    return "fast";
  return "public";
}

__regargs static AreaT *MemPoolAlloc(ULONG byteSize, ULONG attributes) {
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
  
    Panic("[Mem] Failed to allocate %s memory pool of %ldkB!\n",
          MemoryName(attributes), KB(byteSize));
  } 

  area->succ = NULL;
  area->attributes = attributes | MEMF_PUBLIC;
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
  fast = MemPoolAlloc(__fastmem, MEMF_FAST);

  fast->succ = chip;
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

static inline BlockT *BlockAfter(BlockT *blk) {
  return (APTR)blk + abs(blk->size);
}

static __regargs AreaT *FindAreaOf(APTR memoryBlock) {
  AreaT *area = fast;

  while (area && OUTSIDE(memoryBlock, area))
    area = area->succ;

  return area;
}

static __regargs BOOL CheckBlockInternal(BlockT *blk, AreaT *area) {
  if (!blk)
    return TRUE;
  /* block marker damaged ? */
  if (blk->magic != BLK_MAGIC)
    return FALSE;
  /* block size damaged ? */
  if (OUTSIDE(BlockAfter(blk), area))
    return FALSE;
  if (blk->size < 0)
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

  Log("%s Area $%lx - $%lx of %s memory (%ldkB free)\n",
      msg, (LONG)area->lower, (LONG)area->upper, 
      MemoryName(area->attributes), KB(area->freeMem));

  do {
    if (blk->size < 0) {
      Log(" $%lx U %6ld\n", (LONG)blk, -blk->size);
    } else {
      Log(" $%lx F %6ld ($%lx, $%lx)\n",
          (LONG)blk, blk->size, (LONG)blk->prev, (LONG)blk->next);
    }

    if (!CheckBlockInternal(blk, area))
      Panic("%s Block at $%lx damaged!\n", msg, (LONG)blk);

    blk = BlockAfter(blk);
  } while ((APTR)blk < area->upper);
}

__regargs void MemDebug(ULONG attributes) {
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

static __regargs LONG MemLargest(AreaT *area) {
  BlockT *curr = area->first;
  LONG size = 0;

  while (curr) {
    CheckBlock(curr, area, "[MemLargest]");
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

static BlockT *FindBlockForward(AreaT *area, LONG size) {
  BlockT *blk = area->first;

  while (blk) {
    CheckBlock(blk, area, "[MemAlloc]");
    if (blk->size >= size)
      break;
    blk = blk->next;
  }

  return blk;
}

static BlockT *FindBlockReverse(AreaT *area, LONG size) {
  BlockT *blk = area->last;

  while (blk) {
    CheckBlock(blk, area, "[MemAlloc]");
    if (blk->size >= size)
      break;
    blk = blk->prev;
  }

  return blk;
}

static void SplitBlockForward(BlockT *blk, AreaT *area, LONG size) {
  BlockT *new = (BlockT *)((APTR)blk + size);

  new->magic = BLK_MAGIC;
  new->size = blk->size - size;
  new->prev = blk;
  new->next = blk->next;
  (new->next ? new->next->prev : area->last) = new;

  blk->size = size;
  blk->next = new;
}

static BlockT *SplitBlockReverse(BlockT *blk, AreaT *area, LONG size) {
  BlockT *new = (BlockT *)((APTR)blk + (blk->size - size));

  new->magic = BLK_MAGIC;
  new->size = size;
  new->prev = blk;
  new->next = blk->next;
  (new->next ? new->next->prev : area->last) = new;

  blk->size = blk->size - size;
  blk->next = new;

  return new;
}

static void AllocBlock(BlockT *blk, AreaT *area) {
  area->freeMem -= blk->size;

  /* Take the block out of the free list. */
  (blk->prev ? blk->prev->next : area->first) = blk->next;
  (blk->next ? blk->next->prev : area->last) = blk->prev;
  blk->prev = NULL;
  blk->next = NULL;
  blk->size = -blk->size;

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
}

__regargs APTR MemAlloc(ULONG byteSize, ULONG attributes) {
  ULONG size = align(offsetof(BlockT, data) + byteSize, BLK_UNIT);
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
    Log("[MemAlloc] Failed to allocate %ldB of %s memory.\n",
        byteSize, MemoryName(attributes));
    MemDebug(attributes);
    exit(10);
  }

  /* Split the block if can. */
  if (blk->size >= size + sizeof(BlockT)) {
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
    (blk->next) ? (pred->next->prev) : (area->last) = pred;
    blk = pred;
  }

  /* Check if can merge with succeeding block. */
  if (BlockAfter(blk) == blk->next) {
    BlockT *succ = blk->next;

    blk->size += succ->size;
    blk->next = succ->next;
    (succ->next) ? (blk->next->prev) : (area->last) = blk;
  }
}

__regargs void MemFree(APTR memoryBlock) {
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
      Panic("[MemFree] block $%lx not found!\n", (LONG)memoryBlock);
    }
  }
}

__regargs void MemResize(APTR memoryBlock, ULONG byteSize) {
  AreaT *area = FindAreaOf(memoryBlock);

  if (area) {
    BlockT *blk = (BlockT *)(memoryBlock - offsetof(BlockT, data));
    LONG blkSize = -blk->size;
    LONG size = align(offsetof(BlockT, data) + byteSize, BLK_UNIT);

    CheckBlock(blk, area, "[MemResize]");
    CheckBlock(area->first, area, "[MemResize]");
    CheckBlock(area->last, area, "[MemResize]");

    if (size < blkSize) {
      ULONG leftover = blkSize - size;

      /* shrink block if unused memory at the end of block is large enough */
      if (leftover >= sizeof(BlockT)) {
        BlockT *new = (APTR)blk + size;

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
        Log("[MemResize] Failed to resize $%lx to %ldB.\n",
            (LONG)blk, byteSize);
        MemDebugInternal(area, "[MemResize]");
        exit(10);
      }
    }
  } else {
    Panic("[MemResize] block $%lx not found!\n", (LONG)memoryBlock);
  }
}

__regargs LONG MemTypeOf(APTR address) {
  if (INSIDE(address, chip))
    return MEMF_CHIP;
  if (INSIDE(address, fast))
    return MEMF_FAST;
  return -1;
}
