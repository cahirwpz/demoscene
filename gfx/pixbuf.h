#ifndef __GFX_PIXBUF_H__
#define __GFX_PIXBUF_H__

#include "std/types.h"
#include "gfx/palette.h"
#include "std/fp16.h"

#define PIXBUF_GRAY  0
#define PIXBUF_CLUT  1
#define PIXBUF_RGB24 2

#define PIXBUF_TRANSPARENT 1

typedef struct PixBuf {
  uint16_t type;
  uint16_t flags;
  uint8_t  *data;
  size_t   width, height;
  size_t   uniqueColors;  /* stores number of unique colors in the image */

  /* valid only in GRAY or CLUT mode */
  uint8_t  baseColor;
  uint8_t  lastColor;

  uint8_t fgColor;
  uint8_t bgColor;
} PixBufT;

PixBufT *NewPixBuf(uint16_t type, size_t width, size_t height);
PixBufT *NewPixBufFromFile(const StrT fileName);

void PixBufClear(PixBufT *pixbuf);
bool PixBufSetTransparent(PixBufT *pixbuf, bool transparent);
void PixBufRemap(PixBufT *pixbuf, PaletteT *palette);

void PutPixel(PixBufT *pixbuf asm("a0"), int x asm("a1"), int y asm("d1"),
              int c asm("d0"));
int GetPixel(PixBufT *pixbuf asm("a0"), int x asm("d0"), int y asm("d1"));

void PutPixelRGB(PixBufT *pixbuf asm("a0"), int x asm("a1"), int y asm("d1"),
                 RGB c asm("d0"));
RGB GetPixelRGB(PixBufT *pixbuf asm("a0"), int x asm("d0"), int y asm("d1"));

int GetFilteredPixel(PixBufT *pixbuf asm("a0"),
                     Q16T x asm("d0"), Q16T y asm("d1"));

#endif
