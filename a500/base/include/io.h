#ifndef __IO_H__
#define __IO_H__

#include "common.h"

typedef struct File FileT;

#define SEEK_SET -1
#define SEEK_CUR  0

#define IOF_BUFFERED 1

FileT *OpenFile(CONST STRPTR path asm("d1"), UWORD flags asm("d0"));
void CloseFile(FileT *fh asm("a0"));
LONG GetFileSize(CONST STRPTR path asm("d1"));
BOOL FileRead(FileT *fh asm("a0"), APTR buf asm("d2"), ULONG size asm("d3"));
BOOL FileSeek(FileT *fh asm("a0"), LONG pos asm("d2"), LONG mode asm("d3"));
LONG GetCursorPos(FileT *fh asm("a0"));

__regargs APTR LoadFile(CONST STRPTR path, ULONG memoryFlags);

#endif
