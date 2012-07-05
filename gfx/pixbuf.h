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

void PutPixel(PixBufT *pixbuf asm("a0"),
              size_t x asm("d1"), size_t y asm("d2"), uint32_t c asm("d0"));
uint32_t GetPixel(PixBufT *pixbuf asm("a0"),
                  size_t x asm("d1"), size_t y asm("d2"));

#endif
