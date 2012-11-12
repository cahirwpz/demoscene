#include <proto/dos.h> 

#include "iff.h"

__regargs BOOL OpenIff(IffFileT *iff, const char *filename) {
  iff->fh = Open(filename, MODE_OLDFILE);

  if (ReadStruct(iff->fh, &iff->header) &&
      (iff->header.magic == ID_FORM))
    return 1;

  Close(iff->fh);
  return 0;
}

__regargs BOOL ReadChunk(IffFileT *iff) {
  return ReadStruct(iff->fh, &iff->chunk);
}

__regargs void SkipChunk(IffFileT *iff) {
  /* Skip an extra byte if the lenght of a chunk is odd. */
  (void)Seek(iff->fh, (iff->chunk.length + 1) & ~1, OFFSET_CURRENT);
}

__regargs void CloseIff(IffFileT *iff) {
  Close(iff->fh);
}
