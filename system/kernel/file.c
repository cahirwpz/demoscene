#include <system/file.h>
#include <system/errno.h>

struct File {
  FileOpsT *ops;
};

int NoWrite(FileT *f __unused, const void *buf __unused, u_int nbyte __unused) {
  return ENOTSUP;
}

int NoSeek(FileT *f __unused, int offset __unused, int whence __unused) {
  return ENOTSUP;
}

int FileRead(FileT *f asm("a0"), void *buf asm("a1"), u_int nbyte asm("d0")) {
  return f->ops->read(f, buf, nbyte);
}

int FileWrite(FileT *f asm("a0"), const void *buf asm("a1"),
              u_int nbyte asm("d0")) {
  return f->ops->write(f, buf, nbyte);
}

int FileSeek(FileT *f asm("a0"), int offset asm("d0"), int whence asm("d1")) {
  return f->ops->seek(f, offset, whence);
}

void FileClose(FileT *f asm("a0")) {
  f->ops->close(f);
}
