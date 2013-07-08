#include "gfx/blit.h"

static void PixBufBlitNormal(uint8_t *dst asm("a0"), uint8_t *src asm("a1"),
                             size_t width asm("d2"), size_t height asm("d3"),
                             size_t sstride asm("d4"), size_t dstride asm("d5"))
{
  int16_t y = height - 1;

  do {
    int16_t x = width - 1;

    do {
      *dst++ = *src++;
    } while (--x != -1);

    src += sstride;
    dst += dstride;
  } while (--y >= 0);
}

static void PixBufBlitTransparent(uint8_t *dst asm("a0"), uint8_t *src asm("a1"),
                                  size_t width asm("d2"), size_t height asm("d3"),
                                  size_t sstride asm("d4"), size_t dstride asm("d5"))
{
  int16_t y = height - 1;

  do {
    int16_t x = width - 1;

    do {
      uint8_t c = *src++;

      if (c != 0)
        *dst = c;

      dst++;
    } while (--x != -1);

    src += sstride;
    dst += dstride;
  } while (--y >= 0);
}

static inline void ScaleLine(uint8_t *dst, uint8_t *src, int w, const int du2, bool check) {
  const int sx = sign(w);
  const int dx = abs(w);

  int error = du2 - dx;

  w = dx;

  do {
    uint8_t c = *src;

    if ((!check) || (c != 0))
      *dst = *src;

    while (error >= 0) {
      src++;
      error -= 2 * dx;
    }

    dst += sx;
    error += du2;
  } while (--w);
}

/* Uses variant of Bresenham algorithm. */
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

    int error = dv2 - dy;

    dst += ((h > 0) ? (y) : (y - h - 1)) * dstBuf->width;
    dst += (w > 0) ? (x) : (x - w - 1);

    h = dy;

    do {
      ScaleLine(dst, src, w, du2, srcBuf->flags & PIXBUF_TRANSPARENT);

      while (error >= 0) {
        src += srcBuf->width;
        error -= 2 * dy;
      }

      dst += sy;
      error += dv2;
    } while (--h);
  }
}

void PixBufBlit(PixBufT *dbuf, int x, int y,
                PixBufT *sbuf, const RectT *srect)
{
  int sx, sy, w, h;

  if (srect) {
    sx = srect->x;
    sy = srect->y;
    w = min(srect->w, sbuf->width);
    h = min(srect->h, sbuf->height);
  } else {
    sx = 0;
    sy = 0;
    w = sbuf->width;
    h = sbuf->height;
  }

  w = min(w, dbuf->width);
  h = min(h, dbuf->height);

  /* clipping */
  if (x < 0) {
    w += x; sx -= x; x = 0;
  }
  
  if (y < 0) {
    h += y; sy -= y; y = 0;
  }

  if (x + w > dbuf->width)
    w = dbuf->width - x;

  if (y + h > dbuf->height)
    h = dbuf->height - y;

  /* blit */
  if (w > 0 && h > 0) {
    size_t sstride = sbuf->width - w;
    size_t dstride = dbuf->width - w;

    uint8_t *src = &sbuf->data[sy * sbuf->width + sx];
    uint8_t *dst = &dbuf->data[y * dbuf->width + x];

    switch (sbuf->flags) {
      case PIXBUF_TRANSPARENT:
        PixBufBlitTransparent(dst, src, w, h, sstride, dstride);
        break;

      default:
        PixBufBlitNormal(dst, src, w, h, sstride, dstride);
        break;
    } 
  }
}
