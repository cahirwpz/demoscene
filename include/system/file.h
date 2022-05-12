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

#ifndef __STRUCT_FILE
#define __STRUCT_FILE
#endif

struct File {
  FileOpsT *ops;
  __STRUCT_FILE;
};

/* These behave like read/write/lseek known from UNIX */
static inline int FileRead(FileT *f, void *buf, u_int nbyte) {
  return f->ops->read(f, buf, nbyte);
}

static inline int FileWrite(FileT *f, const void *buf, u_int nbyte) {
  return f->ops->write(f, buf, nbyte);
}

static inline int FileSeek(FileT *f, int offset, int whence) {
  return f->ops->seek(f, offset, whence);
}

static inline void FileClose(FileT *f) {
  f->ops->close(f);
}

void FilePutChar(FileT *f, char c);
int FileGetChar(FileT *f);
void FilePrintf(FileT *f, const char *fmt, ...);

FileT *MemOpen(const void *buf, u_int nbyte);

#endif
