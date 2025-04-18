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
