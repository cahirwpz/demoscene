#include "std/math.h"
#include "std/memory.h"
#include "std/fp16.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

void GenerateDirectionTables(Q16T *dxs, Q16T *dys) {
  size_t i;

  for (i = 0; i < 256; i++) {
    float radians = i * 2 * M_PI / 256;

    CastFloatQ16(&dxs[i], -sin(radians));
    CastFloatQ16(&dys[i], -cos(radians));
  }
}

void DirectionalBlur(PixBufT *dst, PixBufT *src, PixBufT *map, int radius) {
  uint8_t *d = dst->data;
  uint8_t *m = src->data;
  Q16T *dxs = NewTable(Q16T, 256);
  Q16T *dys = NewTable(Q16T, 256);
  size_t x, y, i, j;

  GenerateDirectionTables(dxs, dys);

  for (y = 0, i = 0; y < src->height; y++) {
    for (x = 0; x < src->width; x++, i++) {
      size_t direction = m[i];

      Q16T dx = dxs[direction];
      Q16T dy = dys[direction];

      Q16T px, py;

      int value = 0;

      CastIntQ16(&px, x);
      CastIntQ16(&py, y);

      for (j = 0; j <= radius; j++) {
        value += GetFilteredPixel(src, px, py);

        IAddQ16(&px, dx);
        IAddQ16(&py, dy);
      }

      d[i] = value / (radius + 1);
    }
  }

  MemUnref(dxs);
  MemUnref(dys);
}
