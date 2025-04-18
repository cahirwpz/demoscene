#ifndef __SYSTEM_FILE_H__
#define __SYSTEM_FILE_H__

#include <cdefs.h>
#include <types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_NONBLOCK 1

typedef struct File FileT;

#ifdef _SYSTEM
typedef int (*FileReadT)(FileT *f, void *buf, u_int nbyte);
typedef int (*FileWriteT)(FileT *f, const void *buf, u_int nbyte);
typedef int (*FileSeekT)(FileT *f, int offset, int whence);
typedef void (*FileCloseT)(FileT *f);

typedef struct {
  FileReadT read;
  FileWriteT write;
  FileSeekT seek;
  FileCloseT close;
} FileOpsT;

int NoWrite(FileT *f, const void *buf, u_int nbyte);
int NoSeek(FileT *f, int offset, int whence);
#endif

/* These behave like read/write/lseek known from UNIX */
int FileRead(FileT *file, void *buf, u_int nbyte);
int FileWrite(FileT *file, const void *buf, u_int nbyte);
int FileSeek(FileT *file, int offset, int whence);
void FileClose(FileT *file);

void FilePutChar(FileT *f, char c);
int FileGetChar(FileT *f);
void FilePrintf(FileT *f, const char *fmt, ...);

#endif /* !__SYSTEM_FILE_H__ */
