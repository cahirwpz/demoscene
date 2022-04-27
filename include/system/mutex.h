#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <types.h>
#include <system/queue.h>

struct Task;

typedef struct Mutex {
  volatile struct Task *owner;
  TAILQ_HEAD(, Task) waitList;
} MutexT;

static inline void MutexInit(MutexT *mtx) {
  mtx->owner = NULL;
  TAILQ_INIT(&mtx->waitList);
}

#define MUTEX(name)                                                            \
  MutexT name = (MutexT) {                                                     \
    .owner = NULL, .waitList = TAILQ_HEAD_INITIALIZER(name.waitList)           \
  }

void MutexLock(MutexT *mtx);
void MutexUnlock(MutexT *mtx);

#endif /* !__MUTEX_H__ */
