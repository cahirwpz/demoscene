#include <proto/dos.h> 

#include "common.h"
#include "iff.h"

#define ReadStruct(fh, ptr) (Read(fh, ptr, sizeof(*(ptr))) == sizeof(*(ptr)))

__regargs BOOL OpenIff(IffFileT *iff, const char *filename) {
  iff->fh = Open(filename, MODE_OLDFILE);

  if (!iff->fh) {
    Log("File '%s' missing.\n", filename);
    return FALSE;
  }

  if (ReadStruct(iff->fh, &iff->header) &&
      (iff->header.magic == ID_FORM))
    return TRUE;

  Close(iff->fh);
  return FALSE;
}

__regargs BOOL ParseChunk(IffFileT *iff) {
  return ReadStruct(iff->fh, &iff->chunk);
}

__regargs BOOL ReadChunk(IffFileT *iff, APTR ptr) {
  if (Read(iff->fh, ptr, iff->chunk.length) == iff->chunk.length) {
    /* Skip an extra byte if the lenght of a chunk is odd. */
    if (iff->chunk.length & 1)
      (void)Seek(iff->fh, 1, OFFSET_CURRENT);

    return TRUE;
  }

  return FALSE;
}

__regargs void SkipChunk(IffFileT *iff) {
  /* Skip an extra byte if the lenght of a chunk is odd. */
  (void)Seek(iff->fh, (iff->chunk.length + 1) & ~1, OFFSET_CURRENT);
}

__regargs void CloseIff(IffFileT *iff) {
  Close(iff->fh);
}
