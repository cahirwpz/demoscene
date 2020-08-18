#include <stdarg.h>
#include <stdio.h>
#include <file.h>

int FileRead(FileT *f, void *buf, size_t nbyte) {
  return f->ops->read ? f->ops->read(f, buf, nbyte) : -1;
}

int FileWrite(FileT *f, const void *buf, size_t nbyte) {
  return f->ops->write ? f->ops->write(f, buf, nbyte) : -1;
}

int FileSeek(FileT *f, long offset, int whence) {
  return f->ops->seek ? f->ops->seek(f, offset, whence) : -1;
}

void FileClose(FileT *f) {
  if (f->ops->close)
    f->ops->close(f);
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
