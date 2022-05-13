#ifndef __FILE_H__
#define __FILE_H__

#include <cdefs.h>
#include <types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_NONBLOCK 1

typedef struct File FileT;

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

#include <system/syscall.h>

/* These behave like read/write/lseek known from UNIX */
SYSCALL3(FileRead, int, FileT *, file, a0, void *, buf, a1, u_int, nbyte, d0);
SYSCALL3(FileWrite, int, FileT *, file, a0, const void *, buf, a1,
         u_int, nbyte, d0);
SYSCALL3(FileSeek, int, FileT *, file, a0, int, offset, d0, int, whence, d1);
SYSCALL1NR(FileClose, FileT *, file, a0);

void FilePutChar(FileT *f, char c);
int FileGetChar(FileT *f);
void FilePrintf(FileT *f, const char *fmt, ...);

#endif
