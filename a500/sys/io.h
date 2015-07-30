#ifndef __IO_H__
#define __IO_H__

#include "common.h"

typedef LONG FileT;

#define SEEK_SET -1
#define SEEK_CUR  0
#define SEEK_END  1

void Print(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

__regargs APTR LoadFile(CONST STRPTR path, ULONG memoryFlags);
FileT OpenFile(CONST STRPTR path asm("d1"));
void CloseFile(FileT fh asm("d1"));
LONG GetFileSize(CONST STRPTR path asm("d1"));
BOOL FileRead(FileT fh asm("d1"), APTR buf asm("d2"), LONG size asm("d3"));
LONG FileSeek(FileT fh asm("d1"), LONG pos asm("d2"), LONG mode asm("d3"));

#endif
