#ifndef __MEMORY_H__
#define __MEMORY_H__

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

#ifdef _SYSTEM
void MemCheck(int verbose);
u_int MemAvail(u_int attributes);
void AddMemory(void *ptr, u_int byteSize, u_int attributes);
#endif

#include <system/syscall.h>

SCARG2(MemAlloc, void *, u_int, byteSize, d0, u_int, attributes, d1);
SCARG2(MemResize, void *, void *, memoryBlock, a0, u_int, byteSize, d0);
SCARG1NR(MemFree, void *, memoryBlock, a0);

#endif
