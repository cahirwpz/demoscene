#include "gfx/blit.h"
#include "std/debug.h"

__attribute__((regparm(4))) static void
PixBufBlitNormal(uint8_t *dst, uint8_t *src,
                 const int width, const int height,
                 const int sstride, const int dstride)
{
  int y = height;

  do {
    int x = width;

    do {
      *dst++ = *src++;
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}

__attribute__((regparm(4))) static void
PixBufBlitAdditive(uint8_t *dst, uint8_t *src,
                   const int width, const int height,
                   const int sstride, const int dstride)
{
  int y = height;

  do {
    int x = width;

    do {
      uint8_t a = *dst;
      uint8_t b = *src++;

      *dst++ = a + b;
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}

__attribute__((regparm(4))) static void
PixBufBlitAdditiveClip(uint8_t *dst, uint8_t *src,
                       const int width, const int height,
                       const int sstride, const int dstride)
{
  int y = height;

  do {
    int x = width;

    do {
      uint8_t a = *dst;
      uint8_t b = *src++;

      if (a < (uint8_t)~b)
        *dst++ = a + b;
      else
        *dst++ = 255;
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}


__attribute__((regparm(4))) static void
PixBufBlitSubstractive(uint8_t *dst, uint8_t *src,
                       const int width, const int height,
                       const int sstride, const int dstride)
{
  int y = height;

  do {
    int x = width;

    do {
      uint8_t a = *dst;
      uint8_t b = *src++;

      *dst++ = a - b;
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}

__attribute__((regparm(4))) static void
PixBufBlitSubstractiveClip(uint8_t *dst, uint8_t *src,
                           const int width, const int height,
                           const int sstride, const int dstride)
{
  int y = height;

  do {
    int x = width;

    do {
      uint8_t a = *dst;
      uint8_t b = *src++;

      if (a > b)
        *dst++ = a - b;
      else
        *dst++ = 0;
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}

void PixBufBlitTransparent(uint8_t *dst asm("a0"),
                           uint8_t *src asm("a1"),
                           const int width asm("d0"),
                           const int height asm("d1"),
                           const int sstride asm("d2"), 
                           const int dstride asm("d3"));

#if 0
__attribute__((regparm(4))) static void
PixBufBlitTransparent(uint8_t *dst, uint8_t *src,
                      const int width, const int height,
                      const int sstride, const int dstride)
{
  int y = height;

  do {
    int x = width;

    do {
      uint8_t c = *src++;

      if (c != 0)
        *dst++ = c;
      else
        dst++;
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}
#endif

__attribute__((regparm(4))) static void 
PixBufBlitColorMap(uint8_t *dst, uint8_t *src,
                   const int width, const int height,
                   const int sstride, const int dstride,
                   uint8_t *cmap, const int shift)
{
  int y = height;

  do {
    int x = width;

    do {
      int shade = shift + (*src++);

      if (shade < 0)
        shade = 0;
      if (shade > 255)
        shade = 255;

      *dst++ = cmap[(*dst << 8) | shade];
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
}

__attribute__((regparm(4))) static void 
PixBufBlitColorFunc(uint8_t *dst, uint8_t *src,
                    const int width, const int height,
                    const int sstride, const int dstride,
                    uint8_t *cfunc)
{
  int y = height;

  do {
    int x = width;

    do {
      *dst++ = cfunc[*src++];
    } while (--x);

    src += sstride;
    dst += dstride;
  } while (--y);
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
  ASSERT(srcBuf->mode == BLIT_NORMAL || srcBuf->mode == BLIT_TRANSPARENT,
         "Blit mode (%d) not supported.", srcBuf->mode);

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
      ScaleLine(dst, src, w, du2, srcBuf->mode == BLIT_TRANSPARENT);

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

    switch (sbuf->mode) {
      case BLIT_NORMAL:
        PixBufBlitNormal(dst, src, w, h, sstride, dstride);
        break;

      case BLIT_TRANSPARENT:
        PixBufBlitTransparent(dst, src, w, h, sstride, dstride);
        break;

      case BLIT_ADDITIVE:
        PixBufBlitAdditive(dst, src, w, h, sstride, dstride);
        break;

      case BLIT_ADDITIVE_CLIP:
        PixBufBlitAdditiveClip(dst, src, w, h, sstride, dstride);
        break;

      case BLIT_SUBSTRACTIVE:
        PixBufBlitSubstractive(dst, src, w, h, sstride, dstride);
        break;

      case BLIT_SUBSTRACTIVE_CLIP:
        PixBufBlitSubstractiveClip(dst, src, w, h, sstride, dstride);
        break;

      case BLIT_COLOR_MAP:
        PixBufBlitColorMap(dst, src, w, h, sstride, dstride,
                           sbuf->blit.cmap.data, sbuf->blit.cmap.shift);
        break;

      case BLIT_COLOR_FUNC:
        PixBufBlitColorFunc(dst, src, w, h, sstride, dstride,
                            sbuf->blit.cfunc.data);
        break;
    } 
  }
}
