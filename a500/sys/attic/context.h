#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"

typedef struct Context {
  ULONG d0, d1, d2, d3, d4, d5, d6, d7;
  ULONG a0, a1, a2, a3, a4, a5, a6, sp;
  ULONG pc;
  UWORD sr;
} ContextT;

__regargs void GetContext(ContextT *ctx);
__regargs void SetContext(const ContextT *ctx) __attribute__((noreturn));
__regargs void SwapContext(ContextT *octx, const ContextT *nctx) __attribute__((noreturn));

#endif
