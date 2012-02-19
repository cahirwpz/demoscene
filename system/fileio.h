#ifndef __SYSTEM_FILEIO_H__
#define __SYSTEM_FILEIO_H__

#include <exec/memory.h>
#include "std/types.h"

void *ReadFileToCustomMemory(const char *fileName, uint32_t memFlags);
void *ReadFileSimple(const char *fileName);

#define RSC_CHIPMEM_FILE(NAME, FILENAME) { \
  void *_alloc() { return ReadFileToCustomMemory(FILENAME, MEMF_CHIP); } \
  AddLazyRscSimple(NAME, _alloc, (FreeFuncT)MemFree); \
}

#endif
