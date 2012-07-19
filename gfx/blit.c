#include "gfx/blit.h"

void PixBufBlitNormal(PixBufT *dstBuf asm("a0"),
                      size_t x asm("d0"), size_t y asm("d1"),
                      PixBufT *srcBuf asm("a1"))
{
  size_t stride = dstBuf->width - srcBuf->width;

  uint8_t *src = srcBuf->data;
  uint8_t *dst = &dstBuf->data[y * dstBuf->width + x];

  y = srcBuf->height;

  do {
    x = srcBuf->width;

    do {
      *dst++ = *src++;
    } while (--x);

    dst += stride;
  } while (--y);
}

void PixBufBlitTransparent(PixBufT *dstBuf asm("a0"),
                           size_t x asm("d0"), size_t y asm("d1"),
                           PixBufT *srcBuf asm("a1"))
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
    } while (--x);

    dst += stride;
  } while (--y);
}

static inline void ScaleLine(uint8_t *dst, uint8_t *src, int w, const int du2, bool check, uint8_t transparency) {
  const int sx = sign(w);
  const int dx = abs(w);

  int e = du2 - dx;

  w = dx;

  do {
    uint8_t c = *src;

    if ((!check) || (c != transparency))
      *dst = *src;

    while (e >= 0) {
      src++;
      e -= 2 * dx;
    }

    dst += sx;
    e += du2;
  } while (--w);
}

void PixBufBlitScaled(PixBufT *dstBuf, size_t x, size_t y, int w, int h,
                      PixBufT *srcBuf)
{
  if (w && h) {
    const int du2 = 2 * srcBuf->width;
    const int dv2 = 2 * srcBuf->height;

    const int sy = sign(h) * dstBuf->width;
    const int dy = abs(h);

    uint8_t *src = srcBuf->data;
    uint8_t *dst = dstBuf->data;

    int e = dv2 - dy;


    dst += ((h > 0) ? (y) : (y - h - 1)) * dstBuf->width;
    dst += (w > 0) ? (x) : (x - w - 1);

    h = dy;

    do {
      ScaleLine(dst, src, w, du2, srcBuf->flags & PIXBUF_TRANSPARENT, srcBuf->baseColor);

      while (e >= 0) {
        src += srcBuf->width;
        e -= 2 * dy;
      }

      dst += sy;
      e += dv2;
    } while (--h);
  }
}

void PixBufBlit(PixBufT *dstBuf, size_t x, size_t y, PixBufT *srcBuf)
{
  if (srcBuf->flags & PIXBUF_TRANSPARENT) {
    PixBufBlitTransparent(dstBuf, x, y, srcBuf);
  } else {
    PixBufBlitNormal(dstBuf, x, y, srcBuf);
  }
}
