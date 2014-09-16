#include <proto/exec.h>

#include "memory.h"

APTR MemAlloc(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  APTR ptr = AllocMem(byteSize, attributes);

  if (!ptr)
    exit();

  return ptr;
}

void MemFree(APTR memoryBlock asm("a1"), ULONG byteSize asm("d0")) {
  if (memoryBlock)
    FreeMem(memoryBlock, byteSize);
}

typedef struct {
  ULONG size;
  UBYTE data[0];
} MemBlockT;

APTR MemAllocAuto(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  MemBlockT *mb = NULL;

  if ((mb = AllocMem(byteSize + sizeof(MemBlockT), attributes))) {
    mb->size = byteSize;
  } else {
    exit();
  }

  return mb->data;
}

void MemFreeAuto(APTR memoryBlock asm("a1")) {
  if (memoryBlock) {
    MemBlockT *mb = (MemBlockT *)((ULONG *)memoryBlock - 1);
    FreeMem(mb, mb->size + sizeof(MemBlockT));
  }
}
