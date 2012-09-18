#include "std/debug.h"
#include "txtgen/convolution2d.h"

static inline void ClampAndPutPixel(PixBufT *dst, size_t x, size_t y, int32_t value) {
  int last = dst->lastColor - dst->baseColor;

  if (value < 0)
    value = 0;
  if (value > last)
    value = last;

  dst->data[x + y * dst->width] = value + dst->baseColor;
}

void Convolution2D(PixBufT *dst, PixBufT *src, CvltKernelT *kernel) {
  size_t m = (kernel->n + 1) / 2;

  size_t height = dst->height;
  size_t y;

  ASSERT((dst->width == src->width) && (dst->height == src->height),
         "Buffers must have same size!");

  ASSERT((dst->baseColor == src->baseColor) && (dst->lastColor == src->lastColor),
         "Buffers must have same color information!");

  for (y = 0; y < height; y++) {
    size_t minJ = (y < m - 1) ? (m - 1 - y) : 0;
    size_t maxJ = (height - y < m) ? (m - 1 + (height - y)) : kernel->n;

    size_t width = dst->width;
    size_t x;

    for (x = 0; x < width; x++) {
      size_t minI = (x < m - 1) ? (m - 1 - x) : 0;
      size_t maxI = (width - x < m) ? (m - 1 + (width - x)) : kernel->n;

      int sum = 0;

      int py = y + minJ - m;
      int px = x + minI - m;
      int i, j;

      for (j = minJ; j < maxJ; j++, py++)
        for (i = minI; i < maxI; i++, px++) {
          uint8_t pixel = src->data[px + py * width] - dst->baseColor;

          sum += pixel * kernel->matrix[i + j * kernel->n];
        }

      ClampAndPutPixel(dst, x, y, kernel->k * sum);
    }
  }
}
