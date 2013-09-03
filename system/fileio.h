#ifndef __SYSTEM_FILEIO_H__
#define __SYSTEM_FILEIO_H__

#include "std/types.h"

bool FileReadable(const char *fileName);
char *AbsPath(const char *fileName);
void *ReadFileSimple(const char *fileName);
char *ReadTextSimple(const char *fileName);
void WriteFileSimple(const char *fileName, PtrT data, size_t length);

#endif
