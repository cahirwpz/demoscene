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
void *MemAlloc(u_int byteSize, u_int attributes);
void *MemResize(void *memoryBlock, u_int byteSize);
void MemFree(void *memoryBlock);
void AddMemory(void *ptr, u_int byteSize, u_int attributes);
#else
static inline void *MemAlloc(u_int byteSize, u_int attributes) {
  register u_int _d0 asm("d0") = byteSize;
  register u_int _d1 asm("d1") = attributes;
  void *_rv;
  asm volatile ("jsr 192:W"
                : "=d" (_rv)
                : "0" (_d0), "d" (_d1)
                : "d1", "a0", "a1", "cc", "memory");
  return _rv;
}

static inline void *MemResize(void *memoryBlock, u_int byteSize) {
  register void *_a0 asm("a0") = memoryBlock;
  register u_int _d0 asm("d0") = byteSize;
  void *_rv;
  asm volatile ("jsr 198:W"
                : "=d" (_rv)
                : "0" (_d0), "a" (_a0)
                : "d1", "a1", "cc", "memory");
  return _rv;
}

static inline void MemFree(void *memoryBlock) {
  register void *_a0 asm("a0") = memoryBlock;
  asm volatile ("jsr 204:W"
                : 
                : "a" (_a0)
                : "d0", "d1", "a1", "cc", "memory");
}
#endif

#endif
