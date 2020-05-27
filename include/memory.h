#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "types.h"

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
#define MEMF_LARGEST (1L << 17)
#endif

#ifndef MEMF_REVERSE
#define MEMF_REVERSE (1L << 18)
#endif

__regargs void MemDebug(u_int attributes);
__regargs int MemAvail(u_int attributes);
__regargs int MemUsed(u_int attributes);

__regargs void *MemAlloc(u_int byteSize, u_int attributes);
__regargs void MemResize(void *memoryBlock, u_int byteSize);
__regargs void MemFree(void *memoryBlock);
__regargs int MemTypeOf(void *address);

void InitMemory(void);
void KillMemory(void);

#endif
