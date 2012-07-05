#include <math.h>

#include "std/memory.h"
#include "gfx/pixbuf.h"

static int *CreateSinTable(int n, int sines, int phase) {
  int *table = NewTable(int, n);
  float c = M_PI * sines / n;
  int i;

  for (i = 0; i < n; i++) {
    float radian = c * (i + phase);

    table[i] = __builtin_sinf(radian) * 255.0f + 255.0f;
  }

  return table;
}

/*
 * Generate plasma-like pattern.
 *
 * @param xsines: int [0..255]
 * @param ysines: int [0..255]
 * @param xphase: int [0..255]
 * @param yphase: int [0..255]
 */
void GeneratePlasma(PixBufT *pixbuf, size_t xsines, size_t ysines, size_t xphase, size_t yphase)
{
  size_t w = pixbuf->width;
  size_t h = pixbuf->height;
  uint8_t *data = pixbuf->data;
  size_t x, y;

  {
    int *tabx = CreateSinTable(w, xsines, xphase);
    int *taby = CreateSinTable(h, ysines, yphase);

    for (y = 0; y < h; y++)
      for (x = 0; x < w; x++)
        *data++ = (tabx[x] + taby[y]) >> 2;

    MemUnref(tabx);
    MemUnref(taby);
  }
}
