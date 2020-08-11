#ifndef __IO_H__
#define __IO_H__

#include "common.h"

typedef struct File FileT;

#define SEEK_SET -1
#define SEEK_CUR  0

#define IOF_BUFFERED 1

FileT *OpenFile(const char *path, u_short flags);
void CloseFile(FileT *fh);
int GetFileSize(const char *path);
bool FileRead(FileT *fh, void *buf, u_int size);
bool FileSeek(FileT *fh, int pos, int mode);
int GetCursorPos(FileT *fh);

void *LoadFile(const char *path, u_int memoryFlags);

void InitFloppyIO(void);
void KillFloppyIO(void);

#endif
