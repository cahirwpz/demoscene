#include <string.h>
#include <proto/exec.h>

#include "std/memory.h"

void *g_new(size_t n) {
  void *p = NULL;

  if (n) {
    if (!(p = AllocVec(n, MEMF_PUBLIC)))
      _exit();
  }

  return p;
}

void *g_new0(size_t n) {
  void *p = NULL;

  if (n) {
    if (!(p = AllocVec(n, MEMF_PUBLIC|MEMF_CLEAR)))
      _exit();
  }

  return p;
}

void *g_memdup(const void *p, size_t s) {
  return memcpy(g_new(s), p, s);
}

char *g_strdup(const char *s) {
  return g_memdup(s, strlen(s) + 1);
}

void g_free(void *p) {
  FreeVec(p);
}
