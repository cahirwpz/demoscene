#include "common.h"
#include "io.h"
#include "iff.h"

__regargs BOOL OpenIff(IffFileT *iff, CONST STRPTR filename) {
  iff->fh = OpenFile(filename);

  if (!iff->fh) {
    Log("File '%s' missing.\n", filename);
    return FALSE;
  }

  if (FileRead(iff->fh, &iff->header, sizeof(IffHeaderT)) &&
      (iff->header.magic == ID_FORM))
    return TRUE;

  CloseFile(iff->fh);
  return FALSE;
}

__regargs BOOL ParseChunk(IffFileT *iff) {
  return FileRead(iff->fh, &iff->chunk, sizeof(IffChunkT));
}

__regargs BOOL ReadChunk(IffFileT *iff, APTR ptr) {
  if (FileRead(iff->fh, ptr, iff->chunk.length)) {
    /* Skip an extra byte if the lenght of a chunk is odd. */
    if (iff->chunk.length & 1)
      (void)FileSeek(iff->fh, 1, SEEK_CUR);

    return TRUE;
  }

  return FALSE;
}

__regargs void SkipChunk(IffFileT *iff) {
  /* Skip an extra byte if the lenght of a chunk is odd. */
  (void)FileSeek(iff->fh, (iff->chunk.length + 1) & ~1, SEEK_CUR);
}

__regargs void CloseIff(IffFileT *iff) {
  CloseFile(iff->fh);
}
