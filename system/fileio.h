#ifndef __SYSTEM_FILEIO_H__
#define __SYSTEM_FILEIO_H__

#include "std/types.h"

bool InitFileIo();
PtrT ReadFileToChipMem(const StrT fileName);
PtrT ReadFileSimple(const StrT fileName);

#endif
