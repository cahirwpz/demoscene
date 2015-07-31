#ifndef __IFF_H__
#define __IFF_H__

#include "io.h"

#define MAKE_ID(a,b,c,d) \
        ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

#define ID_FORM MAKE_ID('F', 'O', 'R', 'M')
#define ID_BODY MAKE_ID('B', 'O', 'D', 'Y')

typedef struct IffHeader {
  LONG magic;
  LONG length;
  LONG type;
} IffHeaderT;

typedef struct IffChunk {
  LONG type;
  LONG length;
} IffChunkT;

typedef struct IffFile {
  FileT *file;
  IffHeaderT header;
  IffChunkT chunk;
} IffFileT;

__regargs BOOL OpenIff(IffFileT *iff, CONST STRPTR filename);
__regargs BOOL ParseChunk(IffFileT *iff);
__regargs BOOL ReadChunk(IffFileT *iff, APTR ptr);
__regargs void SkipChunk(IffFileT *iff);
__regargs void CloseIff(IffFileT *iff);

#endif
