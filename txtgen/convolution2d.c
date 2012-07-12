#include "std/debug.h"
#include "txtgen/convolution2d.h"

static inline void ClampAndPutPixel(PixBufT *dst, size_t x, size_t y, int32_t value) {
  if (value < 0)
    value = 0;
  if (value > dst->colors - 1)
    value = dst->colors - 1;

  dst->data[x + y * dst->width] = value + dst->baseColor;
}

void Convolution2D(PixBufT *dst, PixBufT *src, CvlKernelT *kernel) {
  size_t m = (kernel->n + 1) / 2;

  int16_t height = dst->height;
  size_t y;

  ASSERT((dst->width == src->width) && (dst->height == src->height),
         "Buffers must have same size!");

  ASSERT((dst->baseColor == src->baseColor) && (dst->colors == src->colors),
         "Buffers must have same color information!");

  for (y = 0; y < height; y++) {
    size_t minJ = (y < m - 1) ? (m - 1 - y) : 0;
    size_t maxJ = (height - y < m) ? (m - 1 + (height - y)) : kernel->n;

    size_t width = dst->width;
    size_t x;

    for (x = 0; x < width; x++) {
      size_t minI = (x < m - 1) ? (m - 1 - x) : 0;
      size_t maxI = (width - x < m) ? (m - 1 + (width - x)) : kernel->n;

      float sum = 0;

      int16_t py = y + minJ - m;
      int16_t px = x + minI - m;
      int16_t i, j;

      for (j = minJ; j < maxJ; j++, py++)
        for (i = minI; i < maxI; i++, px++) {
          uint8_t pixel = src->data[px + py * width] - dst->baseColor;

          sum += pixel * kernel->matrix[i + j * kernel->n];
        }

      ClampAndPutPixel(dst, x, y, kernel->k * sum);
    }
  }
}
