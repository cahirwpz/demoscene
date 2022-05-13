#ifndef __SYSTEM_FILESYS_H__
#define __SYSTEM_FILESYS_H__

#include <types.h>
#include <system/syscall.h>

struct File;

#ifdef _SYSTEM
void InitFileSys(struct File *dev);
void KillFileSys(void);
#endif

SYSCALL1(OpenFile, struct File *, const char *, path, a0);

#endif /* !__SYSTEM_FILESYS_H__ */
