#ifndef __SYSTEM_LOCK_H__
#define __SYSTEM_LOCK_H__

typedef struct Lock LockT;

LockT *NewLock();

void LockShared(LockT *lock);
void LockExclusive(LockT *lock);
void Unlock(LockT *lock);

#endif
