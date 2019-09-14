#ifndef __IO_H__
#define __IO_H__

#include "common.h"

typedef struct File FileT;

#define SEEK_SET -1
#define SEEK_CUR  0

#define IOF_BUFFERED 1

FileT *OpenFile(const char *path asm("d1"), u_short flags asm("d0"));
void CloseFile(FileT *fh asm("a0"));
int GetFileSize(const char *path asm("d1"));
bool FileRead(FileT *fh asm("a0"), void *buf asm("d2"), u_int size asm("d3"));
bool FileSeek(FileT *fh asm("a0"), int pos asm("d2"), int mode asm("d3"));
int GetCursorPos(FileT *fh asm("a0"));

#if USE_IO_DOS
void Print(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
#endif

__regargs void *LoadFile(const char *path, u_int memoryFlags);

#endif
