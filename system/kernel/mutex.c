#include <debug.h>
#include <system/mutex.h>
#define _TASK_PRIVATE
#include <system/task.h>

void MutexLock(MutexT *mtx) {
  TaskT *tsk = CurrentTask;
  IntrDisable();
  while (mtx->owner) {
    tsk->state = TS_BLOCKED;
    TAILQ_INSERT_TAIL(&mtx->waitList, tsk, node);
    TaskYield();
  }
  mtx->owner = tsk;
  IntrEnable();
}

void MutexUnlock(MutexT *mtx) {
  TaskT *tsk;
  IntrDisable();
  if (mtx->owner != CurrentTask)
    PANIC();
  mtx->owner = NULL;
  if ((tsk = TAILQ_FIRST(&mtx->waitList))) {
    TAILQ_REMOVE(&mtx->waitList, tsk, node);
    ReadyAdd(tsk);
  }
  IntrEnable();
}
