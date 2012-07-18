#include "std/math.h"
#include "std/memory.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

static WaveDescT minusSine = { 1, 1.0f, 0.5f };
static WaveDescT minusCosine = { 1, 1.0f, 0.75f };

void DirectionalBlur(PixBufT *dst, PixBufT *src, PixBufT *map, int radius) {
  Q16T *dxs = CalcSineTableQ16(256, &minusSine);
  Q16T *dys = CalcSineTableQ16(256, &minusCosine);
  size_t x, y, i;

  radius++;

  for (y = 0, i = 0; y < src->height; y++) {
    for (x = 0; x < src->width; x++, i++) {
      size_t direction = src->data[i];

      Q16T dx = dxs[direction];
      Q16T dy = dys[direction];

      Q16T px = CastIntQ16(x);
      Q16T py = CastIntQ16(y);

      int value = 0;

      size_t j;

      for (j = 0; j < radius; j++) {
        value += GetFilteredPixel(src, px, py);

        px = AddQ16(px, dx);
        py = AddQ16(py, dy);
      }

      dst->data[i] = value / radius;
    }
  }

  MemUnref(dxs);
  MemUnref(dys);
}
