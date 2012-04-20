#include <string.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/memory.h"

PtrT MemNew(size_t n) {
  PtrT p = NULL;

  if (n) {
    p = AllocVec(n, MEMF_PUBLIC);

    if (!p)
      PANIC("AllocVec(%ld) failed.", n);
  }

  return p;
}

PtrT MemNew0(size_t n) {
  PtrT p = NULL;

  if (n) {
    p = AllocVec(n, MEMF_PUBLIC|MEMF_CLEAR);

    if (!p)
      PANIC("AllocVec(%ld) failed.", n);
  }

  return p;
}

PtrT MemDup(const void *p, size_t s) {
  return memcpy(MemNew(s), p, s);
}

char *StrDup(const char *s) {
  return MemDup(s, strlen(s) + 1);
}

void MemFree(PtrT p) {
  FreeVec(p);
}
