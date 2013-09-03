#undef MEMDEBUG

#include <proto/exec.h>
#include <exec/memory.h>

#include "std/debug.h"
#include "std/memory.h"

void *MainPool = NULL;

void InitMemory() {
  MainPool = CreatePool(MEMF_PUBLIC, 4096, 2048);
}

void KillMemory() {
  DeletePool(MainPool);
}

ADD2INIT(InitMemory, 1);
ADD2EXIT(KillMemory, 1);

/*
 * Memory block is described by following structure:
 *
 * struct MemBlk {
 *   uint32_t size    : 30; // size in longwords or elements iff a table
 *   uint32_t isTable :  1;
 *   uint32_t isTyped :  1;
 * };
 */

#define IS_TYPED  1
#define IS_TABLE  2

/* redefine constants from "exec/memory.h" */
#undef MEM_BLOCKSIZE
#undef MEM_BLOCKMASK

#define MEM_BLOCKSIZE	4
#define MEM_BLOCKMASK	(MEM_BLOCKSIZE - 1)

static inline const TypeT *MemBlkType(PtrT mem) {
  return *(const TypeT **)(mem - 8);
}

static inline uint32_t MemBlkDataExt(PtrT mem) {
  return *(uint32_t *)(mem - 8);
}

static inline uint32_t MemBlkData(PtrT mem) {
  return *(uint32_t *)(mem - 4);
}

static inline bool IsTyped(PtrT mem) {
  return BOOL(MemBlkData(mem) & IS_TYPED);
}

static inline bool IsTable(PtrT mem) {
  return BOOL(MemBlkData(mem) & IS_TABLE);
}

static inline size_t HeaderSize(bool isExt) {
  return isExt ? 8 : 4;
}

static inline PtrT StartAddressOf(PtrT mem) {
  return mem - HeaderSize(MemBlkData(mem) & (IS_TABLE | IS_TYPED));
}

static inline void SizeOf(PtrT mem, size_t *sizePtr, size_t *countPtr) {
  uint32_t blk = MemBlkData(mem);
  size_t size = blk & ~MEM_BLOCKMASK;
  size_t count = 1;

  if (blk & IS_TABLE) {
    count = size / MEM_BLOCKSIZE;
    size = (blk & IS_TYPED) ? MemBlkType(mem)->size : MemBlkDataExt(mem);
  }

  *sizePtr = size;
  *countPtr = count;
}

static inline const TypeT *TypeOf(PtrT mem) {
  return IsTyped(mem) ? MemBlkType(mem) : NULL;
}

/*
 * Memory allocation functions.
 *
 * Caveats:
 * 1) Memory allocation doesn't support block larger that 16MB!
 * 2) Returned memory block is aligned to 4 bytes boundary.
 */

static inline size_t MemBlkSize(size_t size, size_t count, bool isExt) {
  size_t bytes = (size * count + MEM_BLOCKMASK) & ~MEM_BLOCKMASK;

  return bytes + HeaderSize(isExt);
}

static inline PtrT MemBlkInit(PtrT mem, size_t size, bool isTable, bool isTyped) {
  *(uint32_t *)mem = ((isTable ? (size << 2) : (size + MEM_BLOCKMASK)) & ~MEM_BLOCKMASK)
                   | (isTable ? IS_TABLE : 0)
                   | (isTyped ? IS_TYPED : 0);

  return mem + 4;
}

static inline PtrT MemBlkAlloc(size_t n) {
  PtrT ptr = AllocPooled(MainPool, n);

  if (!ptr)
    PANIC("AllocPooled(..., %ld) failed.", n);

  memset(ptr, 0, n);

  return ptr;
}

PtrT MemNewCustom(size_t size, const TypeT *type) {
  PtrT ptr = NULL;

  if (size) {
    size_t bytes = MemBlkSize(size, 1, BOOL(type));

    ptr = MemBlkAlloc(bytes);

    if (type) {
      *(const TypeT **)ptr = type;
      ptr += 4;
    }

    ptr = MemBlkInit(ptr, size, false, BOOL(type));

#ifdef MEMDEBUG
    LOG("Alloc: (block) %08lx %s %ld (orig: %ld)",
        (uint32_t)ptr,
        (type) ? type->name : "?",
        bytes, size);
#endif
  }

  return ptr;
}

PtrT MemNewCustomTable(size_t size, size_t count, const TypeT *type) {
  PtrT ptr = NULL;

  if (size) {
    size_t bytes = MemBlkSize(size, count, true);

    ptr = MemBlkAlloc(bytes);

    if (type)
      *(const TypeT **)ptr = type;
    else
      *(size_t *)ptr = size;

    ptr += 4;

    ptr = MemBlkInit(ptr, count, true, BOOL(type));

#ifdef MEMDEBUG
    LOG("Alloc: (table) %08lx %s %ld (%ld * %ld)",
        (uint32_t)ptr,
        (type) ? type->name : "?",
        bytes, count, size);
#endif
  }

  return ptr;
}

PtrT MemNew(size_t size) {
  return MemNewCustom(size, NULL);
}

PtrT MemNewOfType(const TypeT *type) {
  return MemNewCustom(type->size, type);
}

PtrT MemNewTable(size_t size, size_t count) {
  return MemNewCustomTable(size, count, NULL);
}

PtrT MemNewTableOfType(const TypeT *type, size_t count) {
  return MemNewCustomTable(type->size, count, type);
}

PtrT MemUnref(PtrT mem) {
  if (mem) {
    const TypeT *type = TypeOf(mem);
    size_t size, count, bytes;

    SizeOf(mem, &size, &count);

    bytes = MemBlkSize(size, count, IsTyped(mem) || IsTable(mem));

#ifdef MEMDEBUG
    if (IsTable(mem)) {
      LOG("Free: (table) %08lx %s %ld (%ld * %ld)",
          (uint32_t)mem,
          (type) ? type->name : "?",
          bytes, count, size);
    } else {
      LOG("Free: (block) %08lx %s %ld (orig: %ld)",
          (uint32_t)mem,
          (type) ? type->name : "?",
          bytes, size);
    }
#endif

    if (type && type->free)
      type->free(mem);

    FreePooled(MainPool, StartAddressOf(mem), bytes);

    mem = NULL;
  }

  return mem;
}

size_t TableSize(PtrT mem asm("a0")) {
  uint32_t blk;

  ASSERT(mem, "Null pointer!");
 
  blk = MemBlkData(mem);

  return (blk & IS_TABLE) ? (blk / MEM_BLOCKSIZE) : 1;
}

size_t TableElemSize(PtrT mem asm("a0")) {
  uint32_t blk;

  ASSERT(mem, "Null pointer!");
 
  blk = MemBlkData(mem);

  ASSERT(blk & IS_TABLE, "Expected to get a pointer to a table (%p).", mem);

  return (blk & IS_TYPED) ? (MemBlkType(mem)->size) : MemBlkDataExt(mem);
}

PtrT TableResize(PtrT mem, size_t newCount) {
  const TypeT *type = TypeOf(mem);
  size_t size, count;
  PtrT newMem = NULL;

  ASSERT(IsTable(mem), "Object at $%08lx is not a table.", (uint32_t)newMem);

  SizeOf(mem, &size, &count);

  if (newCount > 0)
    count = newCount;

  newMem = MemNewCustomTable(size, count, type);

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
  size_t size, count;
  PtrT newMem;

  SizeOf(mem, &size, &count);

  if (IsTable(mem)) {
    newMem = TableResize(mem, 0);
  } else {
    newMem = MemNewCustom(size, type);

    if (type && type->copy)
      type->copy(newMem, mem);
    else
      MemCopy(newMem, mem, size);
  }

  return newMem;
}

PtrT MemDup(const void *p, size_t s) {
  return MemCopy(MemNew(s), (APTR)p, s);
}

char *StrDup(const char *s) {
  return MemDup(s, strlen(s) + 1);
}

char *StrNDup(const char *s, size_t l) {
  char *copy;
  int i = 0;

  while (i < l && s[i] != '\0')
    i++;

  copy = MemDup(s, i + 1);
  copy[i] = '\0';
  return copy;
}
