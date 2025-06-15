#include <cdefs.h>
#include <common.h>
#include <debug.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <system/memory.h>
#include <system/mutex.h>
#include <system/task.h>

static MUTEX(MemMtx);

typedef uintptr_t WordT;

#define ALIGNMENT 16
#define CANARY 0xDEADC0DE

/* Free block consists of header BT, pointer to previous and next free block,
 * payload and footer BT. */
#define FREEBLK_SZ (4 * sizeof(WordT))
/* Used block consists of header BT, user memory and canary. */
#define USEDBLK_SZ (2 * sizeof(WordT))

/* Boundary tag flags. */
typedef enum {
  FREE = 0,     /* this block is free */
  USED = 1,     /* this block is used */
  PREVFREE = 2, /* previous block is free */
  ISLAST = 4,   /* last block in an arena */
} BtFlagsT;

/* Stored in payload of free blocks. */
typedef struct Node {
  struct Node *prev;
  struct Node *next;
} NodeT;

/* Structure kept in the header of each managed memory region. */
typedef struct Arena {
  struct Arena *succ; /* next arena */
  NodeT freeList;     /* guard of free block list */
  WordT *end;         /* first address after the arena */
  u_int totalFree;    /* total number of free bytes */
  u_int minFree;      /* minimum recorded number of free bytes */
  u_int attributes;   /* MEMF_* flags */
  /* make sure that user address aligned to ALIGNMENT! */
  WordT start[0];     /* first block in the arena */
} ArenaT;

#define Head(ar) (&(ar)->freeList)

static inline WordT BtSize(WordT *bt) {
  return *bt & ~(USED | PREVFREE | ISLAST);
}

static inline int BtUsed(WordT *bt) {
  return *bt & USED;
}

static inline int BtFree(WordT *bt) {
  return !(*bt & USED);
}

static inline WordT *BtFooter(WordT *bt) {
  return (void *)bt + BtSize(bt) - sizeof(WordT);
}

static inline WordT *BtFromPtr(void *ptr) {
  return (WordT *)ptr - 1;
}

static inline __always_inline void BtMake(WordT *bt, u_int size,
                                          BtFlagsT flags) {
  WordT val = size | flags;
  WordT *ft = (void *)bt + size - sizeof(WordT);
  *bt = val;
  *ft = (flags & USED) ? CANARY : val;
}

static inline int BtHasCanary(WordT *bt) {
  return *BtFooter(bt) == CANARY;
}

static inline BtFlagsT BtGetFlags(WordT *bt) {
  return *bt & (PREVFREE | ISLAST);
}

static inline BtFlagsT BtGetPrevFree(WordT *bt) {
  return *bt & PREVFREE;
}

static inline BtFlagsT BtGetIsLast(WordT *bt) {
  return *bt & ISLAST;
}

static inline void BtClrIsLast(WordT *bt) {
  *bt &= ~ISLAST;
}

static inline void BtClrPrevFree(WordT *bt) {
  *bt &= ~PREVFREE;
}

static inline void BtSetPrevFree(WordT *bt) {
  *bt |= PREVFREE;
}

static inline void *BtPayload(WordT *bt) {
  return bt + 1;
}

static inline WordT *BtNext(WordT *bt) {
  return (void *)bt + BtSize(bt);
}

/* Must never be called on first block in an arena. */
static inline WordT *BtPrev(WordT *bt) {
  WordT *ft = bt - 1;
  return (void *)bt - BtSize(ft);
}

static void BtCheck(WordT *bt) {
  if (!BtHasCanary(bt)) {
    Panic("[Memory] Canary of %p block damaged!", BtPayload(bt));
  }
}

__unused static const char *MemoryName(u_int attributes) {
  if (attributes & MEMF_CHIP)
    return "chip";
  if (attributes & MEMF_FAST)
    return "fast";
  return "public";
}

static inline void ArenaFreeInsert(ArenaT *ar, WordT *bt) {
  NodeT *node = BtPayload(bt);
  NodeT *prev = Head(ar)->prev;

  /* Insert at the end of free block list. */
  node->next = Head(ar);
  node->prev = prev;
  prev->next = node;
  Head(ar)->prev = node;
}

static inline void FreeRemove(WordT *bt) {
  NodeT *node = BtPayload(bt);
  NodeT *prev = node->prev;
  NodeT *next = node->next;
  prev->next = next;
  next->prev = prev;
}

static inline u_int BlockSize(u_int size) {
  return roundup(size + USEDBLK_SZ, ALIGNMENT);
}

#if 0
/* First fit */
static WordT *ArenaFindFit(ArenaT *ar, u_int reqsz) {
  NodeT *n;
  for (n = Head(ar)->next; n != Head(ar); n = n->next) {
    WordT *bt = BtFromPtr(n);
    if (BtSize(bt) >= reqsz)
      return bt;
  }
  return NULL;
}
#else
/* Best fit */
static WordT *ArenaFindFit(ArenaT *ar, u_int reqsz) {
  NodeT *n;
  WordT *best = NULL;
  u_int bestsz = INT_MAX;

  for (n = Head(ar)->next; n != Head(ar); n = n->next) {
    WordT *bt = BtFromPtr(n);
    u_int sz = BtSize(bt);
    if (sz == reqsz)
      return bt;
    if (sz > reqsz && sz < bestsz) {
      best = bt;
      bestsz = sz;
    }
  }

  return best;
}
#endif

static inline void ArenaDecFree(ArenaT *ar, u_int sz) {
  /* Decrease the amount of available memory. */
  ar->totalFree -= sz;
  /* Record the lowest amount of available memory. */
  if (ar->totalFree < ar->minFree)
    ar->minFree = ar->totalFree;
}

static ArenaT *FirstArena;

void AddMemory(void *ptr, u_int size, u_int attributes) {
  ArenaT *ar = (ArenaT *)roundup((uintptr_t)ptr, ALIGNMENT);
  void *end =
      (void *)rounddown((uintptr_t)ptr + size, ALIGNMENT) - sizeof(WordT);
  u_int sz = (uintptr_t)end - (uintptr_t)ar->start;
  WordT *bt = ar->start;

  Assume(ar != NULL);
  Assume(end > (void *)ar->start + FREEBLK_SZ);

  ar->succ = NULL;
  Head(ar)->prev = Head(ar);
  Head(ar)->next = Head(ar);
  ar->end = end;
  ar->totalFree = sz - USEDBLK_SZ;
  ar->minFree = INT_MAX;
  ar->attributes = attributes;
  BtMake(bt, sz, FREE | ISLAST);
  ArenaFreeInsert(ar, bt);

  /* Insert onto arena list. */
  {
    ArenaT **ar_p = &FirstArena;
    while (*ar_p)
      ar_p = &(*ar_p)->succ;
    *ar_p = ar;
  }

  Log("[Memory] Added %s memory at $%08lx - $%08lx (%d KiB)\n",
      MemoryName(attributes), (intptr_t)ar->start, (intptr_t)ar->end,
      ar->totalFree / 1024);
}

static WordT *ArenaMemAlloc(ArenaT *ar, u_int size) {
  u_int reqsz = BlockSize(size);
  WordT *bt;

  MutexLock(&MemMtx);

  bt = ArenaFindFit(ar, reqsz);
  if (bt != NULL) {
    BtFlagsT is_last = BtGetIsLast(bt);
    u_int memsz = reqsz - USEDBLK_SZ;
    /* Mark found block as used. */
    u_int sz = BtSize(bt);
    WordT *next;

    FreeRemove(bt);
    BtMake(bt, reqsz, USED | is_last);
    /* Split free block if needed. */
    next = BtNext(bt);
    if (sz > reqsz) {
      BtMake(next, sz - reqsz, FREE | is_last);
      BtClrIsLast(bt);
      memsz += USEDBLK_SZ;
      ArenaFreeInsert(ar, next);
    } else if (!is_last) {
      /* Nothing to split? Then previous block is not free anymore! */
      BtClrPrevFree(next);
    }
    ArenaDecFree(ar, memsz);
  }

  MutexUnlock(&MemMtx);

  return bt;
}

static void ArenaMemFree(ArenaT *ar, void *ptr) {
  WordT *bt, *next;
  u_int memsz, sz;

  Debug("%s(%p, %p)", __func__, ar, ptr);

  MutexLock(&MemMtx);

  bt = BtFromPtr(ptr);

  /* Is block free and has canary? */
  Assume(BtUsed(bt));
  BtCheck(bt);

  /* Mark block as free. */
  memsz = BtSize(bt) - USEDBLK_SZ;
  sz = BtSize(bt);
  BtMake(bt, sz, FREE | BtGetFlags(bt));

  Debug("bt = %p (size: %u)", bt, sz);

  next = BtNext(bt);
  if (next) {
    if (BtFree(next)) {
      /* Coalesce with next block. */
      FreeRemove(next);
      sz += BtSize(next);
      BtMake(bt, sz, FREE | BtGetPrevFree(bt) | BtGetIsLast(next));
      memsz += USEDBLK_SZ;
    } else {
      /* Mark next used block with prevfree flag. */
      BtSetPrevFree(next);
    }
  }

  /* Check if can coalesce with previous block. */
  if (BtGetPrevFree(bt)) {
    WordT *prev = BtPrev(bt);
    FreeRemove(prev);
    sz += BtSize(prev);
    BtMake(prev, sz, FREE | BtGetIsLast(bt));
    memsz += USEDBLK_SZ;
    bt = prev;
  }

  ar->totalFree += memsz;
  ArenaFreeInsert(ar, bt);

  MutexUnlock(&MemMtx);
}

static void *ArenaMemResize(ArenaT *ar, void *old_ptr, u_int size) {
  void *new_ptr = NULL;
  u_int reqsz, sz;
  WordT *bt, *next;

  bt = BtFromPtr(old_ptr);
  sz = BtSize(bt);
  reqsz = BlockSize(size);

  if (reqsz == sz) {
    /* Same size: nothing to do. */
    return old_ptr;
  }

  MutexLock(&MemMtx);

  if (reqsz < sz) {
    BtFlagsT is_last = BtGetIsLast(bt);
    /* Shrink block: split block and free second one. */
    BtMake(bt, reqsz, USED | BtGetPrevFree(bt));
    next = BtNext(bt);
    BtMake(next, sz - reqsz, USED | is_last);
    ArenaMemFree(ar, BtPayload(next));
    new_ptr = old_ptr;
  } else {
    /* Expand block */
    next = BtNext(bt);
    if (next && BtFree(next)) {
      /* Use next free block if it has enough space. */
      BtFlagsT is_last = BtGetIsLast(next);
      u_int nextsz = BtSize(next);
      if (sz + nextsz >= reqsz) {
        u_int memsz;
        FreeRemove(next);
        BtMake(bt, reqsz, USED | BtGetPrevFree(bt));
        next = BtNext(bt);
        if (sz + nextsz > reqsz) {
          BtMake(next, sz + nextsz - reqsz, FREE | is_last);
          memsz = reqsz - sz;
          ArenaFreeInsert(ar, next);
        } else {
          memsz = nextsz - USEDBLK_SZ;
          BtClrPrevFree(next);
        }
        ArenaDecFree(ar, memsz);
        new_ptr = old_ptr;
      }
    }
  }

  MutexUnlock(&MemMtx);

  Debug("%s(%p, %ld) = %p", __func__, old_ptr, size, new_ptr);
  return new_ptr;
}

#ifdef MEMDEBUG
#define Msg(...) if (verbose) Log(__VA_ARGS__)

static void ArenaCheck(ArenaT *ar, int verbose) {
  WordT *bt = ar->start;
  WordT *prev = NULL;
  NodeT *n;
  int prevfree = 0;
  unsigned freeMem = 0, dangling = 0;

  MutexLock(&MemMtx);

  Msg("Arena: $%08lx - $%08lx [$%x]\n",
      (uintptr_t)ar->start, (uintptr_t)ar->end, ar->attributes);

  for (; bt < ar->end; prev = bt, bt = BtNext(bt)) {
    int flag = !!BtGetPrevFree(bt);
    __unused int is_last = !!BtGetIsLast(bt);
    Msg("$%08lx: [%c%c:%ld] %c\n", (uintptr_t)bt, "FU"[BtUsed(bt)], " P"[flag],
        BtSize(bt), " *"[is_last]);
    if (BtFree(bt)) {
      WordT *ft = BtFooter(bt);
      Assume(*bt == *ft); /* Header and footer do not match? */
      Assume(!prevfree); /* Free block not coalesced? */
      prevfree = 1;
      freeMem += BtSize(bt) - USEDBLK_SZ;
      dangling++;
    } else {
      Assume(flag == prevfree); /* PREVFREE flag mismatch? */
      BtCheck(bt); /* Canary damaged? */
      prevfree = 0;
    }
  }

  Assume(BtGetIsLast(prev)); /* Last block set incorrectly? */
  Assume(freeMem == ar->totalFree); /* Total free memory miscalculated? */

  for (n = Head(ar)->next; n != Head(ar); n = n->next) {
    WordT *bt = BtFromPtr(n);
    Assume(BtFree(bt));
    dangling--;
  }

  Assume(dangling == 0 && "Dangling free blocks!");

  MutexUnlock(&MemMtx);
}
#endif

static ArenaT *ArenaOf(void *ptr) {
  ArenaT *ar;
  for (ar = FirstArena; ar != NULL; ar = ar->succ)
    if (ptr >= (void *)ar->start && ptr < (void *)ar->end)
      break;
  Assume(ar != NULL);
  return ar;
}

void *MemAlloc(u_int size, u_int attributes) {
  WordT *bt = NULL;
  ArenaT *ar;
  void *ptr;

  for (ar = FirstArena; ar != NULL; ar = ar->succ) {
    if (ar->attributes & attributes)
      if ((bt = ArenaMemAlloc(ar, size)))
        break;
  }

  if (bt == NULL) {
    MemCheck(1);
    Panic("[Memory] Failed to allocate %dB of %s memory!",
          size, MemoryName(attributes));
  }

  ptr = BtPayload(bt);
  if (attributes & MEMF_CLEAR)
    bzero(ptr, size);

  Debug("%s(%p, %lu) = %p\n", __func__, ar, size, ptr);

  return ptr;
}

void MemFree(void *p) {
  if (p != NULL)
    ArenaMemFree(ArenaOf(p), p);
}

void *MemResize(void *old_ptr, u_int size) {
  void *new_ptr;
  ArenaT *ar;

  if (size == 0) {
    MemFree(old_ptr);
    return NULL;
  }

  if (old_ptr == NULL)
    return MemAlloc(size, MEMF_PUBLIC);

  ar = ArenaOf(old_ptr);
  if ((new_ptr = ArenaMemResize(ar, old_ptr, size)))
    return new_ptr;

  /* Run out of options - need to move block physically. */
  if ((new_ptr = MemAlloc(size, ar->attributes))) {
    WordT *bt = BtFromPtr(old_ptr);
    Debug("%s(%p, %ld) = %p", __func__, old_ptr, size, new_ptr);
    memcpy(new_ptr, old_ptr, BtSize(bt) - sizeof(WordT));
    MemFree(old_ptr);
    return new_ptr;
  }

  return NULL;
}

#ifdef MEMDEBUG
void MemCheck(int verbose) {
  ArenaT *ar;
  for (ar = FirstArena; ar != NULL; ar = ar->succ)
    ArenaCheck(ar, verbose);
}

u_int MemAvail(u_int attributes) {
  ArenaT *ar;
  u_int avail = 0;
  for (ar = FirstArena; ar != NULL; ar = ar->succ) {
    if (!(ar->attributes & attributes))
      continue;

    if (attributes & MEMF_LARGEST) {
      WordT *bt;

      for (bt = ar->start; bt < ar->end; bt = BtNext(bt)) {
        if (BtFree(bt)) {
          unsigned sz = BtSize(bt) - USEDBLK_SZ;
          if (sz > avail)
            avail= sz;
        }
      }
    } else {
      avail += ar->totalFree;
    }
  }
  return avail;
}
#endif
