#ifndef __GFX_PIXBUF_H__
#define __GFX_PIXBUF_H__

#include "std/types.h"
#include "gfx/palette.h"
#include "std/fp16.h"

#define PIXBUF_GRAY  0
#define PIXBUF_CLUT  1
#define PIXBUF_RGB24 2

typedef enum {
  BLIT_NORMAL,
  BLIT_TRANSPARENT,
  BLIT_ADDITIVE,
  BLIT_SUBSTRACTIVE,
  BLIT_COLOR_MAP,
  BLIT_COLOR_FUNC
} BlitModeT;

typedef struct PixBuf PixBufT;

struct PixBuf {
  /* Basic information. */
  uint16_t type;
  BlitModeT mode;
  uint32_t width, height;

  /* Pixel data. */
  bool ownership; /* false if PixBuf wraps a table */
  uint8_t *data;

  /* Valid only in GRAY or CLUT mode. */
  uint32_t uniqueColors;  /* stores number of unique colors in the image */
  uint8_t baseColor;
  uint8_t lastColor;

  /* Foreground and background color - for drawing. */
  uint8_t fgColor;
  uint8_t bgColor;

  union {
    /* For BLIT_COLOR_MAP mode. */
    struct {
      uint8_t *data;
      int32_t shift;
    } cmap;
    /* For BLIT_COLOR_FUNC mode. */
    struct {
      uint8_t *data;
    } cfunc;
  } blit;
};

PixBufT *NewPixBuf(uint16_t type, size_t width, size_t height);
PixBufT *NewPixBufFromFile(const char *fileName);
PixBufT *NewPixBufWrapper(size_t width, size_t height, uint8_t *data);

void PixBufSwapData(PixBufT *buf1, PixBufT *buf2);
void PixBufCopy(PixBufT *dst, PixBufT *src);
void PixBufClear(PixBufT *pixbuf);
void PixBufSetColorMap(PixBufT *pixbuf, PixBufT *colorMap, int colorShift);
void PixBufSetColorFunc(PixBufT *pixbuf, uint8_t *colorFunc);
BlitModeT PixBufSetBlitMode(PixBufT *pixbuf, BlitModeT mode);
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
