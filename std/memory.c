#include <string.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/memory.h"

/*
 * Memory block can be described with on of the following structures:
 *
 * 1) for simple block (without ClearFunc)
 * struct MemBlk {
 *   uint32_t refCnt : 8;
 *   uint32_t hasClearFunc : 1;  // equals to 0
 *   uint32_t size : 23;
 *   uint8_t data[0];
 * };
 *
 * 2) for extended block (with ClearFunc)
 * struct ExtMemBlk {
 *   ClearFuncT clearFunc;
 *   uint32_t refCnt : 8;
 *   uint32_t hasClearFunc : 1;  // equals to 1
 *   uint32_t size : 23;
 *   uint8_t data[0];
 * };
 *
 * This allows primitive reference counting and memory collection.
 */

static inline bool MemBlkGetSize(PtrT mem) {
  return (*(uint32_t *)(mem - 4)) & 0x7fffff;
}

static inline FreeFuncT MemBlkGetClearFunc(PtrT mem) {
  bool hasClearFunc = BOOL(*(uint8_t *)(mem - 3) & 0x80);

  return hasClearFunc ? *(FreeFuncT *)(mem - 8) : NULL;
}

static inline size_t MemBlkHeaderSize(bool hasClearFunc) {
  return (hasClearFunc ? 8 : 4);
}

static PtrT MemBlkInit(PtrT mem, uint8_t refCnt, size_t size, FreeFuncT func) {
  uint32_t *ptr = (uint32_t *)mem;

  if (func)
    *ptr++ = (uint32_t)func;

  *ptr++ = (refCnt << 24) | (BOOL(func) << 23) | (size & 0x7fffff);

  return ptr;
}

/*
 * Memory allocation & reference counting functions.
 *
 * WARNING: Memory allocation doesn't support block larger that 8MB!
 */

PtrT MemNewInternal(size_t n, uint32_t flags, FreeFuncT func) {
  PtrT p = NULL;

  if (n) {
    n += MemBlkHeaderSize(BOOL(func));
    p = AllocMem(n, flags);

    if (!p)
      PANIC("AllocMem(%ld) failed.", n);
    
    p = MemBlkInit(p, 1, n, func);
  }

  return p;
}

PtrT MemNew(size_t n) {
  return MemNewInternal(n, MEMF_PUBLIC, NULL);
}

PtrT MemNew0(size_t n) {
  return MemNewInternal(n, MEMF_PUBLIC|MEMF_CLEAR, NULL);
}

PtrT RefInc(PtrT mem) {
  uint8_t *refcnt = (uint8_t *)(mem - 4);

  if (*refcnt == 255)
    PANIC("Cannot reference block more than 255 times.");

  (*refcnt)++;

  return mem;
}

PtrT RefDec(PtrT mem) {
  if (mem) {
    uint8_t *refcnt = (uint8_t *)(mem - 4);

    if (*refcnt == 0)
      PANIC("Cannot decrese reference count below zero: %08lx.", *(uint32_t *)(mem - 4));

    (*refcnt)--;

    if (*refcnt == 0) {
      FreeFuncT clear = MemBlkGetClearFunc(mem);
      size_t size = MemBlkGetSize(mem);

      if (clear)
        clear(mem);

      mem -= MemBlkHeaderSize(BOOL(clear));

      FreeMem(mem, size);

      mem = NULL;
    }
  }

  return mem;
}

void MemFree(PtrT mem) {
  (void)RefDec(mem);
}

PtrT MemDup(const void *p, size_t s) {
  return memcpy(MemNew(s), p, s);
}

StrT StrDup(const StrT s) {
  return MemDup(s, strlen(s) + 1);
}
