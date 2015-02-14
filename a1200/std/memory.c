#if 0
#define MEMDEBUG
#endif

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
 * The *BIG* assumption this code makes it that exec.library returns blocks
 * aligned to 8-byte boundary. Look at "exec/memory.h" and MEM_BLOCKSIZE and
 * MEM_BLOCKMASK constants.
 */

#define MEM_BLOCKALIGN(size) (((size) + MEM_BLOCKMASK) & ~MEM_BLOCKMASK)

/* u.type is valid */
#define IS_TYPED 1

/* u.elemSize is valid, size /= MEM_BLOCKSIZE */
#define IS_TABLE 2

/* if this flag and IS_TABLE is set, then this block is a dummy */
#define IS_PADDING 4

typedef struct MemBlk {
  /* Size in bytes (multiple of 8-byte word) or elements (shift right by 3!)
   * iff a table. Note that 3 lower bits are used to mark some facts about the
   * block - mask these before using size value. */
  uint32_t size;

  union {
    TypeT *type;
    uint32_t elemSize;
  } u;

  uint8_t data[0];
} MemBlkT;

/*
 * Memory allocation functions.
 */

static MemBlkT *GetMemBlk(PtrT mem) {
  MemBlkT *blk = (MemBlkT *)(mem - sizeof(MemBlkT));

  ASSERT(mem, "Null pointer!");

  /* Skip padding if a pointer to a table. */
  if ((blk->size & IS_TABLE) && (blk->size & IS_PADDING))
    blk--;

  return blk;
}

static inline TypeT *GetMemBlkType(MemBlkT *blk) {
  return (blk->size & IS_TYPED) ? blk->u.type : NULL;
}

static inline PtrT MemBlkAlloc(uint32_t n) {
  PtrT ptr = AllocPooled(MainPool, n);

  if (!ptr)
    PANIC("AllocPooled(..., %d) failed.", n);

  memset(ptr, 0, n);

  return ptr;
}

PtrT MemNewCustom(uint32_t size, const TypeT *type) {
  PtrT ptr = NULL;

  if (size) {
    uint32_t bytes = sizeof(MemBlkT) + MEM_BLOCKALIGN(size);
    MemBlkT *blk = MemBlkAlloc(bytes);

    blk->size = MEM_BLOCKALIGN(size);

    if (type) {
      blk->size |= IS_TYPED;
      blk->u.type = (TypeT *)type;
    }

    ptr = blk->data;

#ifdef MEMDEBUG
    LOG("Alloc: (block) %p %s %d (orig: %d)",
        ptr, type ? type->name : "?", bytes, size);
#endif
  }

  return ptr;
}

/* Tables are always cache line aligned. */
PtrT MemNewCustomTable(uint32_t elemSize, uint32_t count, const TypeT *type) {
  PtrT ptr = NULL;

  if (count) {
    uint32_t bytes = sizeof(MemBlkT) * 2 + MEM_BLOCKALIGN(elemSize * count);
    MemBlkT *blk = MemBlkAlloc(bytes);

    blk->size = (count * MEM_BLOCKSIZE) | IS_TABLE;

    if (type) {
      blk->size |= IS_TYPED;
      blk->u.type = (TypeT *)type;
    } else {
      blk->u.elemSize = elemSize;
    }

    /* Insert padding if data pointer is not cache line aligned. */
    if ((uint32_t)blk->data & MEM_BLOCKSIZE) {
      blk = (MemBlkT *)blk->data;
      blk->size = IS_PADDING | IS_TABLE;
    }

    ptr = blk->data;

#ifdef MEMDEBUG
    LOG("Alloc: (table) %p %s %d (#%d * %d)",
        ptr, type ? type->name : "?", bytes, count, elemSize);
#endif
  }

  return ptr;
}

PtrT MemNew(uint32_t size) {
  return MemNewCustom(size, NULL);
}

PtrT MemNewOfType(const TypeT *type) {
  return MemNewCustom(type->size, type);
}

PtrT MemNewTable(uint32_t size, uint32_t count) {
  return MemNewCustomTable(size, count, NULL);
}

PtrT MemNewTableOfType(const TypeT *type, uint32_t count) {
  return MemNewCustomTable(type->size, count, type);
}

void MemUnref(PtrT mem) {
  if (mem) {
    MemBlkT *blk = GetMemBlk(mem);
    TypeT *type = GetMemBlkType(blk);
    uint32_t bytes;

    if (blk->size & IS_TABLE) {
      uint32_t count = blk->size / MEM_BLOCKSIZE;
      uint32_t elemSize = 
        (blk->size & IS_TYPED) ? blk->u.type->size : blk->u.elemSize;

      bytes = sizeof(MemBlkT) * 2 + MEM_BLOCKALIGN(count * elemSize);

      /* FIXME: Should I call free routine for each element? */

#ifdef MEMDEBUG
      LOG("Free: (table) %p %s %d (#%d * %d)",
          mem, type ? type->name : "?", bytes, count, elemSize);
#endif
    } else {
      uint32_t size = blk->size & ~MEM_BLOCKMASK;

      bytes = sizeof(MemBlkT) + size;

      if (type && type->free)
        type->free(mem);

#ifdef MEMDEBUG
      LOG("Free: (block) %p %s %d (orig: %d)",
          mem, type ? type->name : "?", bytes, size);
#endif
    }

    FreePooled(MainPool, blk, bytes);
  }
}

uint32_t TableSize(PtrT mem asm("a0")) {
  MemBlkT *blk = GetMemBlk(mem);

  ASSERT(blk->size & IS_TABLE,
         "Expected to get a pointer to a table (%p).", mem);

  return blk->size / MEM_BLOCKSIZE;
}

uint32_t TableElemSize(PtrT mem asm("a0")) {
  MemBlkT *blk = GetMemBlk(mem);

  ASSERT(blk->size & IS_TABLE,
         "Expected to get a pointer to a table (%p).", mem);

  return (blk->size & IS_TYPED) ? blk->u.type->size : blk->u.elemSize;
}

static inline void
TableCopy(PtrT dst, PtrT src, uint32_t elemSize, uint32_t count, TypeT *type)
{
  if (type && type->copy) {
    do {
      type->copy(dst, src);
      src += elemSize;
      dst += elemSize;
    } while (--count);
  } else {
    MemCopy(dst, src, elemSize * count);
  }
}

/* If newCount == 0 then free the table instead of resizing. */
PtrT TableResize(PtrT mem, uint32_t newCount) {
  MemBlkT *blk = GetMemBlk(mem);
  PtrT newMem = NULL;

  ASSERT(blk->size & IS_TABLE,
         "Expected to get a pointer to a table (%p).", mem);

  if (newCount > 0) {
    TypeT *type = GetMemBlkType(blk);
    uint32_t count = blk->size / MEM_BLOCKSIZE;
    uint32_t elemSize = type ? type->size : blk->u.elemSize;

    newMem = MemNewCustomTable(elemSize, newCount, type);

    TableCopy(newMem, mem, elemSize, count, type);

#ifdef MEMDEBUG
    LOG("Resize: (table) %p (%d) -> %p (%d).", mem, count, newMem, newCount);
#endif
  }

  /* Release the old table. */
  MemUnref(mem);

  return newMem;
}

PtrT MemCopy(PtrT dst asm("a1"), const PtrT src asm("a0"), uint32_t n asm("d0")) {
  CopyMem((APTR)src, (APTR)dst, n);
  return dst;
}

PtrT MemClone(PtrT mem) {
  MemBlkT *blk = GetMemBlk(mem);
  TypeT *type = GetMemBlkType(blk);
  PtrT newMem = NULL;

  if (blk->size & IS_TABLE) {
    uint32_t count = blk->size / MEM_BLOCKSIZE;
    uint32_t elemSize = type ? type->size : blk->u.elemSize;

    newMem = MemNewCustomTable(elemSize, count, type);

#ifdef MEMDEBUG
    LOG("Clone: (table) %p (#%d * %d) -> %p.", mem, count, elemSize, newMem);
#endif

    TableCopy(newMem, mem, elemSize, count, type);
  } else {
    uint32_t size = blk->size & ~MEM_BLOCKMASK;

    newMem = MemNewCustom(size, type);

#ifdef MEMDEBUG
    LOG("Clone: (block) %p (%d) -> %p.", mem, size, newMem);
#endif

    if (type && type->copy)
      type->copy(newMem, mem);
    else
      MemCopy(newMem, mem, size);
  }

  return newMem;
}

PtrT MemDup(const void *p, uint32_t s) {
  return MemCopy(MemNew(s), (PtrT)p, s);
}

char *StrDup(const char *s) {
  return MemDup(s, strlen(s) + 1);
}

char *StrNDup(const char *s, uint32_t l) {
  char *copy;
  int i = 0;

  while (i < l && s[i] != '\0')
    i++;

  copy = MemDup(s, i + 1);
  copy[i] = '\0';
  return copy;
}
