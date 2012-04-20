#ifndef __SYSTEM_FILEIO_H__
#define __SYSTEM_FILEIO_H__

#include <exec/memory.h>
#include "std/types.h"

PtrT ReadFileToCustomMemory(const char *fileName, uint32_t memFlags);
PtrT ReadFileSimple(const char *fileName);

#define RSC_CHIPMEM_FILE(NAME, FILENAME) \
  AddRscSimple(NAME, ReadFileToCustomMemory(FILENAME, MEMF_CHIP), \
               (FreeFuncT)MemFree)

#endif
