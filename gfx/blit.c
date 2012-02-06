#include "gfx/blit.h"

void PixBufBlitTransparent(PixBufT *dstBuf, uint16_t x, uint16_t y, PixBufT *srcBuf) {
  size_t stride = dstBuf->width - srcBuf->width;

  uint8_t *dst = &dstBuf->data[y * dstBuf->width + x];
  uint8_t *src = srcBuf->data;

  int srcX, srcY;

  for (srcY = 0; srcY < srcBuf->height; srcY++) {
    for (srcX = 0; srcX < srcBuf->width; srcX++) {
      uint8_t c = *src++;

      if (c != srcBuf->baseColor)
        *dst = c;

      dst++;
    }

    dst += stride;
  }
}
