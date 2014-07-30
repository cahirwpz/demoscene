#include "memory.h"

void *AllocMemSafe(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  void *ptr = AllocMem(byteSize, attributes);

  if (!ptr)
    exit();

  return ptr;
}

typedef struct {
  ULONG size;
  UBYTE data[0];
} MemBlockT;

void *AllocAutoMem(ULONG byteSize asm("d0"), ULONG attributes asm("d1")) {
  MemBlockT *mb;

  if ((mb = AllocMem(byteSize + sizeof(MemBlockT), attributes))) {
    mb->size = byteSize;
    return mb->data;
  }

  return NULL;
}

void FreeAutoMem(void *memoryBlock asm("a1")) {
  MemBlockT *mb = (MemBlockT *)((ULONG *)memoryBlock - 1);
  FreeMem(mb, mb->size + sizeof(MemBlockT));
}
