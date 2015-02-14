#ifndef __STD_EXCEPTION_H__
#define __STD_EXCEPTION_H__

#include <setjmp.h>

#include "std/types.h"

typedef struct Context ContextT;

extern ContextT *_last_ctx;

struct Context {
  ContextT *prev;
  jmp_buf state;
  bool active;
};

void ExceptionInit();
bool ExceptionClear();
void ExceptionFinish();
void ExceptionRaise();

#define TRY \
  ExceptionInit(); \
  if (!setjmp(_last_ctx->state)) \
    for (; ExceptionClear(); ExceptionFinish())

#define RAISE \
  ExceptionRaise()

#define CATCH \
  else \
    for (; ExceptionClear(); ExceptionFinish())

#endif
