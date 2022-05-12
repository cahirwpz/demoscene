#include <system/file.h>
#include <system/errno.h>

int NoWrite(FileT *f __unused, const void *buf __unused, u_int nbyte __unused) {
  return ENOTSUP;
}

int NoSeek(FileT *f __unused, int offset __unused, int whence __unused) {
  return ENOTSUP;
}
