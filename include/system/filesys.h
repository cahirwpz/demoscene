#ifndef __FILESYS_H__
#define __FILESYS_H__

struct File;

#define IOF_BUFFERED 1

struct File *OpenFile(const char *path);
int GetFileSize(const char *path);
void *LoadFile(const char *path, u_int memoryFlags);

void InitFileSys(struct File *dev);
void KillFileSys(void);

#endif
