#ifndef __GFX_PNG_H__
#define __GFX_PNG_H__

#include "std/types.h"
#include "gfx/colors.h"
#include "gfx/pixbuf.h"

#define PNG_GRAYSCALE       0
#define PNG_TRUECOLOR       2
#define PNG_INDEXED         3
#define PNG_GRAYSCALE_ALPHA 4
#define PNG_TRUECOLOR_ALPHA 6

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t colour_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlace_method;
} IhdrT;

typedef struct Idat {
  struct Idat *next;
  int32_t length;
  uint8_t *data;
} IdatT;

typedef struct Plte {
  uint32_t no_colors;
  RGB *colors;
} PlteT;

typedef union {
  struct {
    uint16_t v;
  } type0;
  struct {
    uint16_t r, g, b;
  } type2;
  struct {
    size_t length;
    uint8_t alpha[0];
  } type3;
} TrnsT;

typedef struct {
  IhdrT ihdr;
  IdatT idat;
  PlteT plte;
  TrnsT *trns;
} PngT;

PngT *PngLoadFromFile(const char *filepath);
bool PngDecodeImage(PngT *png, PixBufT *pixbuf);

void LoadPngImage(PixBufT **imgPtr, PaletteT **palPtr, const char *pngFile);

#endif
