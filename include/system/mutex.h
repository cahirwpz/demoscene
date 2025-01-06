#ifndef __SYSTEM_MUTEX_H__
#define __SYSTEM_MUTEX_H__

#include <config.h>
#include <types.h>
#include <system/queue.h>

struct Task;

#ifdef MULTITASK
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
#else
typedef struct Mutex {} MutexT;

#define MUTEX(name) MutexT name = (MutexT){}

static inline void MutexInit(MutexT *mtx) { (void)mtx; }
static inline void MutexLock(MutexT *mtx) { (void)mtx; }
static inline void MutexUnlock(MutexT *mtx) { (void)mtx; }
#endif

#endif /* !__SYSTEM_MUTEX_H__ */
