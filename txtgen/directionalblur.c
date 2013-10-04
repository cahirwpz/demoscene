#include "std/math.h"
#include "std/memory.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

void DirectionalBlur(PixBufT *dst, PixBufT *src, PixBufT *map, int radius) {
  FP16 *dxs = CalcSineTableFP16(256, 1, 1.0f, 0.5f);  /* -sin(x) */
  FP16 *dys = CalcSineTableFP16(256, 1, 1.0f, 0.75f); /* -cos(x) */
  size_t x, y, i;

  radius++;

  for (y = 0, i = 0; y < src->height; y++) {
    for (x = 0; x < src->width; x++, i++) {
      size_t direction = src->data[i];

      FP16 dx = dxs[direction];
      FP16 dy = dys[direction];

      FP16 px = FP16_int(x);
      FP16 py = FP16_int(y);

      int value = 0;

      size_t j;

      for (j = 0; j < radius; j++) {
        value += GetFilteredPixel(src, px, py);

        px = FP16_add(px, dx);
        py = FP16_add(py, dy);
      }

      dst->data[i] = value / radius;
    }
  }

  MemUnref(dxs);
  MemUnref(dys);
}
