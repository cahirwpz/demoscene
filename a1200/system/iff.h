#ifndef __SYSTEM_IFF_H__
#define __SYSTEM_IFF_H__

#include "system/rwops.h"

#define ID_FORM MAKE_ID('F', 'O', 'R', 'M')
#define ID_BODY MAKE_ID('B', 'O', 'D', 'Y')

typedef struct IffChunk {
  uint32_t type;
  uint32_t length;
} IffChunkT;

typedef struct IffHeader {
  uint32_t magic;
  uint32_t length;
  uint32_t type;
} IffHeaderT;

typedef struct IffFile {
  RwOpsT *fh;
  IffHeaderT header;
  IffChunkT chunk;
} IffFileT;

__regargs IffFileT *IffOpen(const char *filename);
__regargs bool IffParseChunk(IffFileT *iff);
__regargs void *IffReadChunk(IffFileT *iff);
__regargs bool IffSkipChunk(IffFileT *iff);

#endif
