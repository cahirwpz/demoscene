#include <debug.h>
#include <string.h>
#include <system/errno.h>
#include <system/file.h>
#include <system/memfile.h>
#include <system/memory.h>

struct File {
  FileOpsT *ops;
  const MemBlockT *blocks;
  u_int offset;
};

static int MemRead(FileT *f, void *buf, u_int nbyte);
static int MemSeek(FileT *f, int offset, int whence);
static void MemClose(FileT *f);

static FileOpsT MemOps = {
  .read = MemRead,
  .write = NoWrite,
  .seek = MemSeek,
  .close = MemClose
};

FileT *MemOpen(const MemBlockT *blocks) {
  FileT *f = MemAlloc(sizeof(FileT), MEMF_PUBLIC);
  f->ops = &MemOps;
  f->blocks = blocks;
  f->offset = 0;
  return f;
}

static void MemClose(FileT *f) {
  MemFree(f);
}

static int MemRead(FileT *f, void *buf, u_int nbyte) {
  const MemBlockT *block = f->blocks;
  u_int offset = f->offset;
  u_int nread = 0;

  Debug("$%p $%p %d+%d", f, buf, f->offset, nbyte);

  for (; block->length && nbyte > 0; block++) {
    u_int todo;

    if (offset > block->length) {
      offset -= block->length;
      continue;
    }

    if (offset + nbyte > block->length) {
      todo = block->length - offset;
    } else {
      todo = nbyte;
    }

    memcpy(buf, block->addr + offset, todo);
    buf += todo;
    nread += todo;
    nbyte -= todo;
    f->offset += todo;

    offset += todo - block->length;
  }

  return nread;
}

static int MemSeek(FileT *f, int offset, int whence) {
  const MemBlockT *block = f->blocks;
  int length;

  for (length = 0; block->length; block++)
    length += block->length;

  if (whence == SEEK_CUR) {
    offset += f->offset;
  } else if (whence == SEEK_END) {
    offset += length;
  } else if (whence != SEEK_SET) {
    return EINVAL;
  }

  if (offset < 0) {
    offset = 0;
  } else if (offset > length) {
    offset = length;
  }

  f->offset = offset;
  return offset;
}
