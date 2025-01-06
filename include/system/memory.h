#ifndef __SYSTEM_MEMORY_H__
#define __SYSTEM_MEMORY_H__

#include <types.h>

#ifndef MEMF_PUBLIC
#define MEMF_PUBLIC (1L << 0)
#endif

#ifndef MEMF_CHIP
#define MEMF_CHIP (1L << 1)
#endif

#ifndef MEMF_FAST
#define MEMF_FAST (1L << 2)
#endif

#ifndef MEMF_CLEAR
#define MEMF_CLEAR (1L << 16)
#endif

#ifndef MEMF_LARGEST
/* MemAvail: return the largest chunk size */
#define MEMF_LARGEST (1L << 17)
#endif

#ifdef _SYSTEM
#if MEMDEBUG
void MemCheck(int verbose);
u_int MemAvail(u_int attributes);
#else
#define MemCheck(_) { (void)0; }
#define MemAvail(_) 0
#endif
void AddMemory(void *ptr, u_int byteSize, u_int attributes);
#endif

void *MemAlloc(u_int byteSize, u_int attributes);
void *MemResize(void *memoryBlock, u_int byteSize);
void MemFree(void *memoryBlock);

#endif /* !__SYSTEM_MEMORY_H__ */
