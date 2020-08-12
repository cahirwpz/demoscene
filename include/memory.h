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

#define BLK_UNIT  8
#define BLK_ALIGN __attribute__((aligned(BLK_UNIT)))

void MemCheck(int verbose);
u_int MemAvail(u_int attributes);

void AddMemory(void *ptr, u_int byteSize, u_int attributes);
void *MemAlloc(u_int byteSize, u_int attributes);
void *MemResize(void *memoryBlock, u_int byteSize);
void MemFree(void *memoryBlock);

#endif
