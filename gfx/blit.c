#include "gfx/blit.h"

static void PixBufBlitNormal(uint8_t *dst asm("a0"), uint8_t *src asm("a1"),
                             size_t width asm("d2"), size_t height asm("d3"),
                             size_t stride asm("d4"))
{
  int16_t y = height - 1;

  do {
    int16_t x = width - 1;

    do {
      *dst++ = *src++;
    } while (--x != -1);

    dst += stride;
  } while (--y >= 0);
}

static void PixBufBlitTransparent(uint8_t *dst asm("a0"), uint8_t *src asm("a1"),
                                  size_t width asm("d2"), size_t height asm("d3"),
                                  size_t stride asm("d4"), uint8_t transparent asm("d5"))
{
  int16_t y = height - 1;

  do {
    int16_t x = width - 1;

    do {
      uint8_t c = *src++;

      if (c != transparent)
        *dst = c;

      dst++;
    } while (--x != -1);

    dst += stride;
  } while (--y >= 0);
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

void PixBufBlit(PixBufT *dstBuf, size_t x, size_t y, PixBufT *srcBuf) {
  size_t stride = dstBuf->width - srcBuf->width;
  size_t w = srcBuf->width;
  size_t h = srcBuf->height;

  uint8_t *src = srcBuf->data;
  uint8_t *dst = &dstBuf->data[y * dstBuf->width + x];

  /* If source image is longer, then copy only visible area. */
  if (y + h > dstBuf->height)
    h = dstBuf->height - y;

  switch (srcBuf->flags) {
    case PIXBUF_TRANSPARENT:
      PixBufBlitTransparent(dst, src, w, h, stride, srcBuf->baseColor);
      break;

    default:
      PixBufBlitNormal(dst, src, w, h, stride);
      break;
  } 
}
