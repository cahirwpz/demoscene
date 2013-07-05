#undef MEMDEBUG

#include <proto/exec.h>

#include "std/debug.h"
#include "std/memory.h"

/*
 * Memory block can be described by following structure:
 *
 * struct MemBlk {
 *   uint32_t refCnt  :  8;
 *   uint32_t size    : 21;  // size in quadwords or elements iff a table
 *   uint32_t unused  :  1;
 *   uint32_t isTable :  1;
 *   uint32_t isTyped :  1;  // equals to 0
 * };
 *
 * This allows primitive reference counting and memory collection.
 */

#define IS_TYPED  1
#define IS_TABLE  2
#define SIZE_MASK 0xfffff8

static inline const TypeT *MemBlkType(PtrT mem) {
  return *(const TypeT **)(mem - 8);
}

static inline uint32_t MemBlkDataExt(PtrT mem) {
  return *(uint32_t *)(mem - 8);
}

static inline uint32_t MemBlkData(PtrT mem) {
  return *(uint32_t *)(mem - 4);
}

static inline int IsTyped(PtrT mem) {
  return MemBlkData(mem) & IS_TYPED;
}

static inline int IsTable(PtrT mem) {
  return MemBlkData(mem) & IS_TABLE;
}

static inline size_t HeaderSize(bool isExt) {
  return isExt ? 8 : 4;
}

static inline PtrT StartAddressOf(PtrT mem) {
  return mem - HeaderSize(MemBlkData(mem) & (IS_TABLE | IS_TYPED));
}

static inline void SizeOf(PtrT mem, size_t *sizePtr, size_t *countPtr) {
  uint32_t blk = MemBlkData(mem);
  size_t size = blk & SIZE_MASK;
  size_t count = 1;

  if (blk & IS_TABLE) {
    count = size / MEM_BLOCKSIZE;

    if (blk & IS_TYPED) {
      size = (MemBlkType(mem))->size;
    } else {
      size = MemBlkDataExt(mem);
    }
  }

  *sizePtr = size;
  *countPtr = count;
}

static inline const TypeT *TypeOf(PtrT mem) {
  return IsTyped(mem) ? MemBlkType(mem) : NULL;
}

/*
 * Memory allocation & reference counting functions.
 *
 * Caveats:
 * 1) Memory allocation doesn't support block larger that 16MB!
 * 2) Returned memory block is aligned to 4 bytes boundary.
 */

static inline size_t MemBlkSize(size_t size, size_t count, bool isExt) {
  size_t bytes = size * count + HeaderSize(isExt);

  return (bytes + MEM_BLOCKMASK) & ~MEM_BLOCKMASK;
}

static inline PtrT MemBlkInit(PtrT mem, size_t size, bool isTable, bool isTyped) {
  *(uint32_t *)mem = (1 << 24)
                   | ((isTable ? (size << 3) : size) & SIZE_MASK)
                   | (isTable ? IS_TABLE : 0)
                   | (isTyped ? IS_TYPED : 0);

  return mem + 4;
}

static inline PtrT MemBlkAlloc(size_t n, uint32_t flags) {
  PtrT ptr = AllocMem(n, flags);

  if (!ptr)
    PANIC("AllocMem(%ld) failed.", n);

  return ptr;
}

PtrT MemNewCustom(size_t size, uint32_t flags, const TypeT *type) {
  PtrT ptr = NULL;

  if (size) {
    size_t bytes = MemBlkSize(size, 1, BOOL(type));

    ptr = MemBlkAlloc(bytes, flags);

    if (type) {
      *(const TypeT **)ptr = type;
      ptr += 4;
    }

    ptr = MemBlkInit(ptr, size, FALSE, BOOL(type));

#ifdef MEMDEBUG
    LOG("Alloc: (block) %08lx %s %ld",
        (uint32_t)ptr,
        (type) ? type->name : "",
        bytes);
#endif
  }

  return ptr;
}

PtrT MemNewCustomTable(size_t size, size_t count, uint32_t flags,
                       const TypeT *type)
{
  PtrT ptr = NULL;

  if (size) {
    size_t bytes = MemBlkSize(size, count, TRUE);

    ptr = MemBlkAlloc(bytes, flags);

    if (type)
      *(const TypeT **)ptr = type;
    else
      *(size_t *)ptr = size;

    ptr += 4;

    ptr = MemBlkInit(ptr, count, TRUE, BOOL(type));

#ifdef MEMDEBUG
    LOG("Alloc: (table) %08lx %s %ld (%ld * %ld)",
        (uint32_t)ptr,
        (type) ? type->name : "",
        bytes,
        size,
        count);
#endif
  }

  return ptr;
}

PtrT MemNew(size_t size) {
  return MemNewCustom(size, MEMF_PUBLIC|MEMF_CLEAR, NULL);
}

PtrT MemNewOfType(const TypeT *type) {
  return MemNewCustom(type->size, MEMF_PUBLIC|MEMF_CLEAR, type);
}

PtrT MemNewTable(size_t size, size_t count) {
  return MemNewCustomTable(size, count, MEMF_PUBLIC|MEMF_CLEAR, NULL);
}

PtrT MemNewTableOfType(const TypeT *type, size_t count) {
  return MemNewCustomTable(type->size, count, MEMF_PUBLIC|MEMF_CLEAR, type);
}

PtrT MemUnref(PtrT mem) {
  if (mem) {
    uint8_t *refcnt = (uint8_t *)(mem - 4);

    if (*refcnt == 0)
      PANIC("Cannot decrese reference count below zero: %08lx.", *(uint32_t *)(mem - 4));

    (*refcnt)--;

    if (*refcnt == 0) {
      const TypeT *type = TypeOf(mem);
      size_t size, count;

      SizeOf(mem, &size, &count);

#ifdef MEMDEBUG
      LOG("Free: (%s) %08lx %s %ld",
          IsTable(mem) ? "table" : "block",
          (uint32_t)mem,
          (type) ? type->name : "",
          MemBlkSize(size, count, BOOL(type)));
#endif

      if (type && type->free)
        type->free(mem);

      FreeMem(StartAddressOf(mem), MemBlkSize(size, count, BOOL(type)));

      mem = NULL;
    }
  }

  return mem;
}

size_t TableSize(PtrT mem asm("a0")) {
  uint32_t blk = MemBlkData(mem);

  return (blk & IS_TABLE) ? (blk & SIZE_MASK) / MEM_BLOCKSIZE : 1;
}

size_t TableElemSize(PtrT mem asm("a0")) {
  uint32_t blk = MemBlkData(mem);

  ASSERT(blk & IS_TABLE, "Expected to get a pointer to a table.");

  return (blk & IS_TYPED) ? (MemBlkType(mem)->size) : MemBlkDataExt(mem);
}

PtrT TableResize(PtrT mem, size_t newCount) {
  const TypeT *type = TypeOf(mem);
  uint32_t flags = TypeOfMem(mem);
  size_t size, count;
  PtrT newMem = NULL;

  ASSERT(IsTable(mem), "Object at $%08lx is not a table.", (uint32_t)newMem);

  SizeOf(mem, &size, &count);

  if (newCount > 0)
    count = newCount;

  newMem = MemNewCustomTable(size, count, flags, type);

  if (type && type->copy) {
    PtrT src = mem;
    PtrT dst = newMem;

    do {
      type->copy(dst, src);
      src += size;
      dst += size;
    } while (--count);
  } else {
    MemCopy(newMem, mem, size * count);
  }

  /* If not cloning, release the old table. */
  if (newCount > 0)
    MemUnref(mem);

  return newMem;
}

PtrT MemCopy(PtrT dst asm("a1"), const PtrT src asm("a0"), size_t n asm("d0")) {
  CopyMem((APTR)src, (APTR)dst, n);
  return dst;
}

PtrT MemClone(PtrT mem) {
  const TypeT *type = TypeOf(mem);
  uint32_t flags = TypeOfMem(mem);
  size_t size, count;
  PtrT newMem;

  SizeOf(mem, &size, &count);

  if (IsTable(mem)) {
    newMem = TableResize(mem, 0);
  } else {
    newMem = MemNewCustom(size, flags, type);

    if (type && type->copy)
      type->copy(newMem, mem);
    else
      MemCopy(newMem, mem, size);
  }

  return newMem;
}

PtrT MemDup(const PtrT p, size_t s) {
  return MemCopy(MemNew(s), p, s);
}

StrT StrDup(const StrT s) {
  return MemDup(s, strlen(s) + 1);
}

StrT StrNDup(const StrT s, size_t l) {
  char *copy;
  int i = 0;

  while (i < l && s[i] != '\0')
    i++;

  copy = MemDup(s, i + 1);
  copy[i] = '\0';
  return copy;
}
