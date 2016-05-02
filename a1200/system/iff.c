#include "system/iff.h"
#include "std/memory.h"

static void IffClose(IffFileT *iff) {
  if (iff->fh) {
    IoClose(iff->fh);
    MemUnref(iff->fh);
  }
}

TYPEDECL(IffFileT, (FreeFuncT)IffClose);

__regargs IffFileT *IffOpen(const char *filename) {
  IffFileT *iff = NewRecord(IffFileT);

  if ((iff->fh = RwOpsFromFile(filename, "r")))
    if (IoRead(iff->fh, &iff->header, sizeof(iff->header)) ==
        sizeof(iff->header) && (iff->header.magic == ID_FORM))
      return iff;

  MemUnref(iff);
  return NULL;
}

__regargs bool IffParseChunk(IffFileT *iff) {
  return IoRead(iff->fh, &iff->chunk, sizeof(iff->chunk))
    == sizeof(iff->chunk);
}

__regargs void *IffReadChunk(IffFileT *iff) {
  void *ptr = MemNew(iff->chunk.length);

  if (IoRead(iff->fh, ptr, iff->chunk.length) == iff->chunk.length) {
    /* Skip an extra byte if the lenght of a chunk is odd. */
    if (iff->chunk.length & 1)
      (void)IoSeek(iff->fh, 1, OFFSET_CURRENT);
    return ptr;
  }

  MemUnref(ptr);
  return NULL;
}

__regargs bool IffSkipChunk(IffFileT *iff) {
  /* Skip an extra byte if the lenght of a chunk is odd. */
  return IoSeek(iff->fh, (iff->chunk.length + 1) & ~1, IO_SEEK_CUR) != -1;
}
