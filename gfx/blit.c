#include "gfx/blit.h"

void PixBufBlitTransparent(PixBufT *dstBuf asm("a0"), size_t x asm("d0"),
                           size_t y asm("d1"), PixBufT *srcBuf asm("a1"))
{
  size_t stride = dstBuf->width - srcBuf->width;

  uint8_t *src = srcBuf->data;
  uint8_t *dst = &dstBuf->data[y * dstBuf->width + x];

  uint8_t transparent = srcBuf->baseColor;

  y = srcBuf->height;

  do {
    x = srcBuf->width;

    do {
      uint8_t c = *src++;

      if (c != transparent)
        *dst = c;

      dst++;
    } while (--x > 0);

    dst += stride;
  } while (--y > 0);
}
