#include <string.h>
#include <proto/exec.h>

#include "system/memory.h"

void *xmalloc(size_t n) {
  void *p = NULL;

  if (n) {
    if (!(p = AllocVec(n, MEMF_PUBLIC)))
      _exit();
  }

  return p;
}

void *xzalloc(size_t n) {
  void *p = NULL;

  if (n) {
    if (!(p = AllocVec(n, MEMF_PUBLIC|MEMF_CLEAR)))
      _exit();
  }

  return p;
}

void *xmemdup(const void *p, size_t s) {
  return memcpy(xmalloc(s), p, s);
}

char *xstrdup(const char *s) {
  return xmemdup(s, strlen(s) + 1);
}

void xfree(void *p) {
  FreeVec(p);
}
