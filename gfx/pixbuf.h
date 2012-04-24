#ifndef __GFX_PIXBUF_H__
#define __GFX_PIXBUF_H__

#include "std/types.h"
#include "gfx/palette.h"

typedef struct PixBuf {
  uint8_t *data;
  uint16_t width, height, colors;
  uint8_t baseColor;
} PixBufT;

PixBufT *NewPixBuf(size_t width, size_t height);
PixBufT *NewPixBufFromFile(const StrT fileName);

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette);

#endif
