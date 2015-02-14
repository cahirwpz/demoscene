#include <exec/semaphores.h>
#include <proto/exec.h>

#include "std/memory.h"
#include "system/lock.h"

struct Lock {
  struct SignalSemaphore semaphore;
};

LockT *NewLock() {
  LockT *lock = NewRecord(LockT);

  InitSemaphore(&lock->semaphore);

  return lock;
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
