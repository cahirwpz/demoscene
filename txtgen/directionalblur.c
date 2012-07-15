#include "std/math.h"
#include "std/memory.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

void GenerateDirectionTables(int *dxs, int *dys) {
  size_t i;

  for (i = 0; i < 256; i++) {
    float radians = i * 2 * M_PI / 256;

    dxs[i] = lroundf(-sin(radians) * 65536);
    dys[i] = lroundf(-cos(radians) * 65536);
  }
}

void DirectionalBlur(PixBufT *dst, PixBufT *src, PixBufT *map, int radius) {
  uint8_t *d = dst->data;
  uint8_t *m = src->data;
  int *dxs = NewTable(int, 256);
  int *dys = NewTable(int, 256);
  size_t x, y, i, j;

  GenerateDirectionTables(dxs, dys);

  for (y = 0, i = 0; y < src->height; y++) {
    for (x = 0; x < src->width; x++, i++) {
      size_t direction = m[i];

      int dx = dxs[direction];
      int dy = dys[direction];

      int px = x << 16;
      int py = y << 16;

      int value = 0;

      for (j = 0; j <= radius; j++) {
        value += GetFilteredPixel(src, px, py);

        px += dx;
        py += dy;
      }

      d[i] = value / (radius + 1);
    }
  }

  MemUnref(dxs);
  MemUnref(dys);
}
