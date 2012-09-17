#ifndef __SYSTEM_FILEIO_H__
#define __SYSTEM_FILEIO_H__

#include "std/types.h"

StrT AbsPath(const StrT fileName);
PtrT ReadFileToChipMem(const StrT fileName);
PtrT ReadFileSimple(const StrT fileName);
void WriteFileSimple(const StrT fileName, PtrT data, size_t length);

#endif
