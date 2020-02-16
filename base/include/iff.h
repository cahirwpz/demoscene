#ifndef __IFF_H__
#define __IFF_H__

#include "io.h"

#define ID_FORM MAKE_ID('F', 'O', 'R', 'M')
#define ID_BODY MAKE_ID('B', 'O', 'D', 'Y')

typedef struct IffHeader {
  int magic;
  int length;
  int type;
} IffHeaderT;

typedef struct IffChunk {
  int type;
  int length;
} IffChunkT;

typedef struct IffFile {
  FileT *file;
  IffHeaderT header;
  IffChunkT chunk;
} IffFileT;

__regargs void OpenIff(IffFileT *iff, const char *filename);
__regargs bool ParseChunk(IffFileT *iff);
__regargs bool ReadChunk(IffFileT *iff, void *ptr);
__regargs void SkipChunk(IffFileT *iff);
__regargs void CloseIff(IffFileT *iff);

#endif
