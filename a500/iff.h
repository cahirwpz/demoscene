#ifndef __IFF_H__
#define __IFF_H__

#include <exec/types.h>
#include <dos/dos.h>

#define ID_FORM 0x464F524D
#define ID_BMHD 0x424D4844
#define ID_CMAP 0x434D4150
#define ID_BODY 0x424F4459

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
  BPTR fh;
  IffHeaderT header;
  IffChunkT chunk;
} IffFileT;

__regargs BOOL OpenIff(IffFileT *iff, const char *filename);
__regargs BOOL ParseChunk(IffFileT *iff);
__regargs BOOL ReadChunk(IffFileT *iff, APTR ptr);
__regargs void SkipChunk(IffFileT *iff);
__regargs void CloseIff(IffFileT *iff);

#endif
