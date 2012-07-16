#ifndef __GFX_PIXBUF_H__
#define __GFX_PIXBUF_H__

#include "std/types.h"
#include "gfx/palette.h"
#include "std/fp16.h"

#define PIXBUF_GRAY  0
#define PIXBUF_CLUT  1
#define PIXBUF_RGB24 2

typedef struct PixBuf {
  uint32_t type;
  uint8_t  *data;
  size_t   width, height;
  size_t   colors;        /* stores number of unique colors in the image */
  uint8_t  baseColor;     /* valid only in GRAY or CLUT mode */
} PixBufT;

PixBufT *NewPixBuf(size_t width, size_t height);
PixBufT *NewPixBufFromFile(const StrT fileName);

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette);

void PutPixel(PixBufT *pixbuf asm("a0"), int x asm("a1"), int y asm("d1"),
              int c asm("d0"));
int GetPixel(PixBufT *pixbuf asm("a0"), int x asm("d0"), int y asm("d1"));

void PutPixelRGB(PixBufT *pixbuf asm("a0"), int x asm("a1"), int y asm("d1"),
                 ColorT c asm("d0"));
ColorT GetPixelRGB(PixBufT *pixbuf asm("a0"),
                   int x asm("d0"), int y asm("d1"));

int GetFilteredPixel(PixBufT *pixbuf asm("a0"),
                     Q16T x asm("d0"), Q16T y asm("d1"));

#endif
