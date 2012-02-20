#include <exec/semaphores.h>
#include <proto/exec.h>

#include "std/memory.h"
#include "system/lock.h"

struct Lock {
  struct SignalSemaphore semaphore;
};

LockT *NewLock() {
  LockT *lock = NEW_S(LockT);

  InitSemaphore(&lock->semaphore);

  return lock;
}

void LockDelete(LockT *lock) {
  DELETE(lock);
}

void LockShared(LockT *lock) {
  ObtainSemaphoreShared(&lock->semaphore);
}

void LockExclusive(LockT *lock) {
  ObtainSemaphore(&lock->semaphore);
}

void Unlock(LockT *lock) {
  ReleaseSemaphore(&lock->semaphore);
}
