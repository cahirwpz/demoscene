#include <proto/dos.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/rwops.h"

static int
FileRead(RwOpsT *stream asm("a0"),
         void *buffer asm("d2"), unsigned int size asm("d3"))
{
  if (stream->opened) {
    int read = min(size, stream->u.memory.stop - stream->u.memory.here);

    if (read) {
      MemCopy(buffer, stream->u.memory.here, read);
      stream->u.memory.here += read;
    }

    return read;
  }

  return -1;
}

static int
FileWrite(RwOpsT *stream asm("a0"),
          const void *buffer asm("d2"), unsigned int size asm("d3"))
{
  if (stream->opened) {
    int written = min(size, stream->u.memory.stop - stream->u.memory.here);

    if (written) {
      MemCopy(stream->u.memory.here, (void *)buffer, written);
      stream->u.memory.here += written;
    }

    return written;
  }

  return -1;
}

static int 
FileSeek(RwOpsT *stream asm("a0"),
         int offset asm("d2"), SeekModeT whence asm("d3"))
{
  if (stream->opened) {
    int oldpos = stream->u.memory.here - stream->u.memory.stop;
    uint8_t *pos = NULL;

    switch (whence) {
      case IO_SEEK_SET:
        pos = stream->u.memory.base + offset;
        break;
      case IO_SEEK_CUR:
        pos = stream->u.memory.here + offset;
        break;
      case IO_SEEK_END:
        pos = stream->u.memory.stop + offset;
        break;
    }

    if (pos < stream->u.memory.base)
      return -1;

    if (pos > stream->u.memory.stop)
      return -1;

    stream->u.memory.here = pos;

    LOG("base = $%p, here = $%p, stop = $%p", 
        stream->u.memory.base,
        stream->u.memory.here,
        stream->u.memory.stop);

    return oldpos;
  }

  return -1;
}

static int FileTell(RwOpsT *stream asm("a0")) {
  if (stream->opened)
    return stream->u.memory.here - stream->u.memory.base;
  return 0;
}

static int FileClose(RwOpsT *stream asm("a0")) {
  MemUnref(stream->u.memory.base);
  stream->opened = false;
  return 0;
}

RwOpsT *RwOpsFromMemory(void *ptr, uint32_t size) {
  RwOpsT *stream = NewRecord(RwOpsT);

  stream->type = IO_MEMORY;
  stream->opened = true;
  stream->u.memory.base = ptr;
  stream->u.memory.here = ptr;
  stream->u.memory.stop = ptr + size;

  stream->read = FileRead;
  stream->write = FileWrite;
  stream->seek = FileSeek;
  stream->tell = FileTell;
  stream->close = FileClose;

  return stream;
}
