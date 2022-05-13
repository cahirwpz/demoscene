#ifndef __SYSTEM_MEMFILE_H__
#define __SYSTEM_MEMFILE_H__

#include <types.h>
#include <system/syscall.h>

struct File;

struct File *MemOpen(const void *buf, u_int nbyte);

#endif /* !__SYSTEM_MEMFILE_H__ */
