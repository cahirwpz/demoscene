#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <file.h>

#define IOF_BUFFERED 1

FileT *OpenFile(const char *path);
int GetFileSize(const char *path);
void *LoadFile(const char *path, u_int memoryFlags);

void InitFileSys(void);
void KillFileSys(void);

#endif
