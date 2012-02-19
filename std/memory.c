#include <string.h>
#include <proto/exec.h>

#include "std/memory.h"

void *MemNew(size_t n) {
  void *p = NULL;

  if (n) {
    if (!(p = AllocVec(n, MEMF_PUBLIC)))
      _exit();
  }

  return p;
}

void *MemNew0(size_t n) {
  void *p = NULL;

  if (n) {
    if (!(p = AllocVec(n, MEMF_PUBLIC|MEMF_CLEAR)))
      _exit();
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
