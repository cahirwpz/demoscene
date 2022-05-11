#include <debug.h>
#include <string.h>
#include <system/errno.h>
#include <system/memory.h>

#define __STRUCT_FILE   \
  const void *buf;      \
  u_int offset;         \
  int length;

#include <system/file.h>

static int MemRead(FileT *f, void *buf, u_int nbyte);
static int MemSeek(FileT *f, int offset, int whence);
static void MemClose(FileT *f);

static FileOpsT MemOps = {
  .read = MemRead,
  .write = NoWrite,
  .seek = MemSeek,
  .close = MemClose
};

FileT *MemOpen(const void *buf, u_int length) {
  FileT *f = MemAlloc(sizeof(FileT), MEMF_PUBLIC);
  f->ops = &MemOps;
  f->buf = buf;
  f->length = length;
  f->offset = 0;
  return f;
}

static void MemClose(FileT *f) {
  MemFree(f);
}

static int MemRead(FileT *f, void *buf, u_int nbyte) {
  int start = f->offset;
  int nread = nbyte;

  Debug("$%p $%p %d+%d", f, buf, f->offset, nbyte);

  if (start + nread > f->length)
    nread = f->length - start;

  memcpy(buf, f->buf + start, nread);
  f->offset += nread;
  return nread;
}

static int MemSeek(FileT *f, int offset, int whence) {
  if (whence == SEEK_CUR) {
    offset += f->offset;
  } else if (whence == SEEK_END) {
    offset += f->length;
  } else if (whence != SEEK_SET) {
    return EINVAL;
  }

  if (offset < 0) {
    offset = 0;
  } else if (offset > f->length) {
    offset = f->length;
  }

  f->offset = offset;
  return offset;
}
