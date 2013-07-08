#include "std/debug.h"
#include "std/memory.h"
#include "gfx/filter.h"

#define DIV_BY_3(x) (((x) * (65536 / 3)) >> 16)

void BlurH3(PixBufT *dstBuf, PixBufT *srcBuf) {
  uint8_t *dst = dstBuf->data;
  uint8_t *prev = srcBuf->data;
  uint8_t *next = srcBuf->data + 2;
  int16_t x, y;

  /*
   * At any given moment keeps a sum of three pixels: previous, current and
   * next. If previous or next are unavailable (possible at the beginning and
   * the end of line) then value of 0 is assumed.
   */
  int16_t c;

  ASSERT(dstBuf->width == srcBuf->width,
         "Width does not match (%ld != %ld)", dstBuf->width, srcBuf->width);
  ASSERT(dstBuf->height == srcBuf->height,
         "Height does not match (%ld != %ld)", dstBuf->height, srcBuf->height);

  y = srcBuf->height;

  do {
    /* give the first pixel special treatment: c[0] = src[0] + src[1] */
    c = prev[0] + prev[1];
    *dst++ = DIV_BY_3(c);

    x = srcBuf->width - 2;

    do {
      /* c[i] = c[i - 1] + (src[i + 1] - src[i - 2]) */
      c += *next++;
      *dst++ = DIV_BY_3(c);
      c -= *prev++;
    } while (--x);

    /* give the last pixel special treatment: c[N] = c[N - 1] - src[N - 2] */
    *dst++ = DIV_BY_3(c);

    prev += 2;
    next += 2;
  } while (--y);
}

void BlurV3(PixBufT *dstBuf, PixBufT *srcBuf) {
  uint8_t *dst = dstBuf->data;
  uint8_t *prev = srcBuf->data;
  uint8_t *next = srcBuf->data + 2 * srcBuf->width;
  int16_t x, y;

  /* at any given moment keeps a sum of three pixels */
  int16_t *line = NewTable(int16_t, srcBuf->width);

  /* initially line is a sum of srcLine[0] and srcLine[1] */
  for (x = 0; x < srcBuf->width; x++) {
    line[x] = srcBuf->data[x] + srcBuf->data[x + srcBuf->width];
    *dst++ = DIV_BY_3(line[x]);
  }

  ASSERT(dstBuf->width == srcBuf->width,
         "Width does not match (%ld != %ld)", dstBuf->width, srcBuf->width);
  ASSERT(dstBuf->height == srcBuf->height,
         "Height does not match (%ld != %ld)", dstBuf->height, srcBuf->height);

  y = srcBuf->height - 2;

  do {
    for (x = 0; x < srcBuf->width; x++) {
      line[x] += *next++;
      *dst++ = DIV_BY_3(line[x]);
      line[x] -= *prev++; 
    }
  } while (--y >= 0);

  for (x = 0; x < srcBuf->width; x++)
    *dst++ = DIV_BY_3(line[x]);

  MemUnref(line);
}
