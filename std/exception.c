#include "std/exception.h"
#include "std/memory.h"

ContextT *_last_ctx = NULL;

/*
 * Creates new exception context.
 */
void ExceptionInit() {
  ContextT *new_ctx = NewRecord(ContextT);

  new_ctx->prev = _last_ctx;
  new_ctx->active = true;

  _last_ctx = new_ctx;
}

/*
 * Removes exception context if marked as inactive.
 */
bool ExceptionClear() {
  if (!_last_ctx->active) {
    ContextT *prev_ctx = _last_ctx->prev;

    MemUnref(_last_ctx);

    _last_ctx = prev_ctx;

    return false;
  }

  return true;
}

/*
 * Marks exception context as inactive.
 */
void ExceptionFinish() {
  _last_ctx->active = false;
}

/*
 * Used to raise an exception.
 */
void ExceptionRaise() {
  if (!_last_ctx)
    abort();

  longjmp(_last_ctx->state, 1);
}
