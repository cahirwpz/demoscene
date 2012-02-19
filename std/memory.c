#include <string.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/memory.h"

void *MemNew(size_t n) {
  void *p = NULL;

  if (n) {
    p = AllocVec(n, MEMF_PUBLIC);

    if (!p)
      PANIC("AllocVec(%d) failed.", n);
  }

  return p;
}

void *MemNew0(size_t n) {
  void *p = NULL;

  if (n) {
    p = AllocVec(n, MEMF_PUBLIC|MEMF_CLEAR);

    if (!p)
      PANIC("AllocVec(%d) failed.", n);
  }

  return p;
}

void *MemDup(const void *p, size_t s) {
  return memcpy(MemNew(s), p, s);
}

char *StrDup(const char *s) {
  return MemDup(s, strlen(s) + 1);
}

void MemFree(void *p) {
  FreeVec(p);
}
