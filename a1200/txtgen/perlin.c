#include "std/random.h"
#include "std/types.h"
#include "gfx/pixbuf.h"

static inline int AddRandom(int value, int size, int32_t *random) {
  value += (RandomInt32(random) & (size - 1)) - (size >> 1);

  return max(min(value, 255), 0);
}

void GeneratePerlinNoise(PixBufT *pixbuf, int32_t *random) {
  size_t size = pixbuf->width;
  size_t n = size / 2;

  PutPixel(pixbuf, 0, 0, RandomInt32(random));
  PutPixel(pixbuf, n, 0, RandomInt32(random));
  PutPixel(pixbuf, 0, n, RandomInt32(random));
  PutPixel(pixbuf, n, n, RandomInt32(random));

  while (n > 1) {
    size_t x, y;
    size_t hn = n / 2;

    for (y = 0; y < size; y += n) {
      for (x = 0; x < size; x += n) {
        int a = GetPixel(pixbuf, x, y);
        int b = GetPixel(pixbuf, x + n, y);
        int c = GetPixel(pixbuf, x, y + n);
        int d = GetPixel(pixbuf, x + n, y + n);

        d = (a + b + c + d) / 4;
        b = (a + b) / 2;
        c = (a + c) / 2;

        PutPixel(pixbuf, x + hn, y, AddRandom(b, hn, random));
        PutPixel(pixbuf, x, y + hn, AddRandom(c, hn, random));
        PutPixel(pixbuf, x + hn, y + hn, AddRandom(d, hn, random));
      }
    }

    n >>= 1;
  }
}
