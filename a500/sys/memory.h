#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <exec/types.h>
#include <exec/memory.h>

LONG MemAvail(ULONG attributes asm("d1"));
LONG MemUsed(ULONG attributes asm("d1"));

APTR MemAlloc(ULONG byteSize asm("d0"), ULONG attributes asm("d1"));
void MemFree(APTR memoryBlock asm("a1"), ULONG byteSize asm("d0"));

APTR MemAllocAuto(ULONG byteSize asm("d0"), ULONG attributes asm("d1"));
void MemFreeAuto(APTR memoryBlock asm("a1"));

#endif
