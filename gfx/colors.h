#ifndef __GFX_COLORS_H__
#define __GFX_COLORS_H__

#include "std/types.h"

typedef struct Color {
  uint8_t r, g, b, alpha;
} ColorT;

typedef struct ColorVector {
  float h, s, l;
} ColorVectorT;

void ColorsInvert(ColorT *dst, ColorT *src, size_t count);
void ColorsAverage(uint8_t *dst, ColorT *src, size_t count);
void ColorsContrast(ColorT *dst, ColorT *src, size_t count);
void ColorsChangeHSL(ColorT *dst, ColorT *src, size_t count, ColorVectorT *d);

#endif
