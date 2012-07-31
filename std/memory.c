#include <string.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/memory.h"

/*
 * Memory block can be described with one of the following structures:
 *
 * 1) for untyped block (without TypeT)
 * struct MemBlk {
 *   uint32_t refCnt      :  8;
 *   uint32_t size        : 22;  // size in quadwords 
 *   uint32_t isTable     :  1;
 *   uint32_t hasTypeDesc :  1;  // equals to 0
 *   uint8_t data[0];
 * };
 *
 * 2) for untyped array block (without TypeT)
 * struct ArrayMemBlk {
 *   uint32_t elemSize;
 *   uint32_t refCnt      :  8;
 *   uint32_t size        : 22;  // size in elements
 *   uint32_t isTable     :  1;  // equals to 1
 *   uint32_t hasTypeDesc :  1;  // equals to 0
 *   uint8_t data[0];
 * };
 *
 * 3) for typed (non-)array block (with TypeT)
 * struct TypedMemBlk {
 *   TypeT    *typeDesc;
 *   uint32_t refCnt      :  8;
 *   uint32_t size        : 22;  // size in quadwords or elements iff a table
 *   uint32_t isTable     :  1;  // 0 or 1
 *   uint32_t hasTypeDesc :  1;  // equals to 1
 *   uint8_t data[0];
 * };
 *
 * This allows primitive reference counting and memory collection.
 */

static inline bool MemBlkGetSize(PtrT mem) {
  return (*(uint32_t *)(mem - 4)) & 0xfffff8;
}

static inline const TypeT *MemBlkGetType(PtrT mem) {
  bool hasTypeDesc = BOOL(*(uint32_t *)(mem - 4) & 1);

  return hasTypeDesc ? (const TypeT *)(*(uint32_t *)(mem - 8)) : NULL;
}

static inline size_t MemBlkHeaderSize(bool hasTypeDesc) {
  return (hasTypeDesc ? 8 : 4);
}

static PtrT MemBlkInit(PtrT mem, uint8_t refCnt, size_t size, const TypeT *type) {
  uint32_t *ptr = (uint32_t *)mem;

  if (type)
    *ptr++ = (uint32_t)type;

  *ptr++ = (refCnt << 24) | (size & 0xfffff8) | (type ? 1 : 0);

  return ptr;
}

/*
 * Memory allocation & reference counting functions.
 *
 * Caveats:
 * 1) Memory allocation doesn't support block larger that 16MB!
 * 2) Returned memory block is aligned to 4 bytes boundary.
 */

PtrT MemNewInternal(size_t n, uint32_t flags, const TypeT *type) {
  PtrT p = NULL;

  if (n) {
    n += MemBlkHeaderSize(BOOL(type));
    n = (n + MEM_BLOCKMASK) & -MEM_BLOCKSIZE;
    p = AllocMem(n, flags);

    if (!p)
      PANIC("AllocMem(%ld) failed.", n);
    
    p = MemBlkInit(p, 1, n, type);
  }

  return p;
}

PtrT MemNew(size_t n) {
  return MemNewInternal(n, MEMF_PUBLIC|MEMF_CLEAR, NULL);
}

PtrT MemNewObject(const TypeT *type) {
  return MemNewInternal(type->size, MEMF_PUBLIC|MEMF_CLEAR, type);
}

PtrT MemRef(PtrT mem) {
  uint8_t *refcnt = (uint8_t *)(mem - 4);

  if (*refcnt == 255)
    PANIC("Cannot reference block more than 255 times.");

  (*refcnt)++;

  return mem;
}

PtrT MemUnref(PtrT mem) {
  if (mem) {
    uint8_t *refcnt = (uint8_t *)(mem - 4);

    if (*refcnt == 0)
      PANIC("Cannot decrese reference count below zero: %08lx.", *(uint32_t *)(mem - 4));

    (*refcnt)--;

    if (*refcnt == 0) {
      const TypeT *type = MemBlkGetType(mem);
      size_t size = MemBlkGetSize(mem);

      if (type && type->free)
        type->free(mem);

      mem -= MemBlkHeaderSize(BOOL(type));

      FreeMem(mem, size);

      mem = NULL;
    }
  }

  return mem;
}

PtrT MemDupGC(PtrT mem, size_t s) {
  PtrT newMem = MemNewInternal(s, TypeOfMem(mem), MemBlkGetType(mem)); 

  return memcpy(newMem, mem, s);
}

PtrT MemDup(const void *p, size_t s) {
  return memcpy(MemNew(s), p, s);
}

StrT StrDup(const StrT s) {
  return MemDup(s, strlen(s) + 1);
}
