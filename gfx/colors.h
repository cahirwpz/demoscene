#ifndef __GFX_COLORS_H__
#define __GFX_COLORS_H__

#include "std/types.h"

typedef struct {
  uint8_t r, g, b;
} RGB;

typedef struct {
  float h, s, l;
} HSL;

void ColorsInvert(RGB *dst, RGB *src, size_t count);
void ColorsAverage(uint8_t *dst, RGB *src, size_t count);
void ColorsContrast(RGB *dst, RGB *src, size_t count);
void ColorsChangeHSL(RGB *dst, RGB *src, size_t count, HSL *d);

#endif
