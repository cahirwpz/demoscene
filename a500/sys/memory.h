#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>

void *AllocMemSafe(ULONG byteSize asm("d0"), ULONG attributes asm("d1"));
void *AllocAutoMem(ULONG byteSize asm("d0"), ULONG attributes asm("d1"));
void FreeAutoMem(void *memoryBlock asm("a1"));

#endif
