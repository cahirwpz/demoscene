#ifndef __GFX_PIXBUF_H__
#define __GFX_PIXBUF_H__

#include "std/types.h"

typedef struct PixBuf {
  uint16_t width, height;
  uint8_t *data;
} PixBufT;

PixBufT *NewPixBuf(int width, int height);
PixBufT *NewPixBufFromFile(const char *fileName, uint32_t memFlags);
void DeletePixBuf(PixBufT *pixbuf);

#endif
