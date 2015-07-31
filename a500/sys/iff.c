#include "common.h"
#include "io.h"
#include "iff.h"

__regargs BOOL OpenIff(IffFileT *iff, CONST STRPTR filename) {
  iff->file = OpenFile(filename);

  if (!iff->file) {
    Log("File '%s' missing.\n", filename);
    return FALSE;
  }

  if (FileRead(iff->file, &iff->header, sizeof(IffHeaderT)) &&
      (iff->header.magic == ID_FORM))
    return TRUE;

  CloseFile(iff->file);
  return FALSE;
}

__regargs BOOL ParseChunk(IffFileT *iff) {
  return FileRead(iff->file, &iff->chunk, sizeof(IffChunkT));
}

__regargs BOOL ReadChunk(IffFileT *iff, APTR ptr) {
  if (FileRead(iff->file, ptr, iff->chunk.length)) {
    /* Skip an extra byte if the lenght of a chunk is odd. */
    if (iff->chunk.length & 1)
      (void)FileSeek(iff->file, 1, SEEK_CUR);

    return TRUE;
  }

  return FALSE;
}

__regargs void SkipChunk(IffFileT *iff) {
  /* Skip an extra byte if the lenght of a chunk is odd. */
  (void)FileSeek(iff->file, (iff->chunk.length + 1) & ~1, SEEK_CUR);
}

__regargs void CloseIff(IffFileT *iff) {
  CloseFile(iff->file);
}
