#include <stdarg.h>
#include <stdio.h>
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

int FileRead(FileT *f, void *buf, u_int nbyte) {
  return f->ops->read(f, buf, nbyte);
}

int FileWrite(FileT *f, const void *buf, u_int nbyte) {
  return f->ops->write(f, buf, nbyte);
}

int FileSeek(FileT *f, int offset, int whence) {
  return f->ops->seek(f, offset, whence);
}

void FileClose(FileT *f) {
  f->ops->close(f);
}

int FileGetChar(FileT *f) {
  char c;
  if (FileRead(f, &c, 1) < 1)
    return -1;
  return c;
}

void FilePutChar(FileT *f, char c) {
  FileWrite(f, &c, 1);
}

void FilePrintf(FileT *f, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  kvprintf((kvprintf_fn_t *)FilePutChar, f, fmt, ap);
  va_end(ap);
}
