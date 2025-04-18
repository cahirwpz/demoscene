#include <debug.h>
#include <system/mutex.h>
#define _TASK_PRIVATE
#include <system/task.h>

void MutexLock(MutexT *mtx) {
  TaskT *tsk = CurrentTask;
  IntrDisable();
  while (mtx->owner) {
    tsk->waitpt = __builtin_return_address(0);
    tsk->state = TS_BLOCKED;
    TAILQ_INSERT_TAIL(&mtx->waitList, tsk, node);
    TaskYield();
    tsk->waitpt = NULL;
  }
  mtx->owner = tsk;
  IntrEnable();
}

void MutexUnlock(MutexT *mtx) {
  TaskT *tsk;
  IntrDisable();
  Assume(mtx->owner == CurrentTask);
  mtx->owner = NULL;
  if ((tsk = TAILQ_FIRST(&mtx->waitList))) {
    TAILQ_REMOVE(&mtx->waitList, tsk, node);
    ReadyAdd(tsk);
    MaybePreempt();
  }
  IntrEnable();
}
