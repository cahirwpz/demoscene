#include "gfx/pixbuf.h"
#include "std/debug.h"
#include "std/fastmath.h"

static inline void AddPixel(PixBufT *pixbuf, int x, int y, uint8_t c) {
  uint8_t *p = &pixbuf->data[x + pixbuf->width * y];
  uint16_t d = *p + c;

  if (d >= 256)
    d = 255;

  *p = d;
}

void DrawLineAA(PixBufT *canvas, float x0, float y0, float x1, float y1) {
  bool steep = fabsf(y1 - y0) > fabsf(x1 - x0);
  float dx, dy;
  float gradient, intery;
  float xend, yend, xgap, yend_i, yend_f;
  int xpxl1, ypxl1, xpxl2, ypxl2, x;
 
  if (steep) {
    swapf(x0, y0);
    swapf(x1, y1);
  }

  if (x0 > x1) {
    swapf(x0, x1);
    swapf(y0, y1);
  }
 
  dx = x1 - x0;
  dy = y1 - y0;
  gradient = dy / dx;

  if (dx == 0.0f)
    return;

  /* Handle first endpoint. */
  xend = truncf(x0 + 0.5f);
  yend = y0 + gradient * (xend - x0);
  xgap = 1.0f - modff(x0 + 0.5f, NULL);
  xgap *= 128.0f;

  /* This will be used in the main loop. */
  yend_f = modff(yend, &yend_i);
  xpxl1 = (int)xend;
  ypxl1 = (int)truncf(yend);

  /* Draw first endpoint. */
  if (steep) {
    AddPixel(canvas, ypxl1, xpxl1, (1.0f - yend_f) * xgap);
    AddPixel(canvas, ypxl1 + 1, xpxl1, yend_f * xgap);
  } else {
    AddPixel(canvas, xpxl1, ypxl1, (1.0f - yend_f) * xgap);
    AddPixel(canvas, xpxl1, ypxl1 + 1, yend_f * xgap);
  }

  /* First y-intersection for the main loop. */
  intery = yend + gradient;

  /* Handle second endpoint. */
  xend = truncf(x1 + 0.5f);
  yend = y1 + gradient * (xend - x1);
  xgap = modff(x1 + 0.5f, NULL);
  xgap *= 128.0f;

  /* This will be used in the main loop. */
  yend_f = modff(yend, &yend_i);
  xpxl2 = (int)xend;
  ypxl2 = (int)yend_i;

  if (steep) {
    /* Draw second endpoint. */
    AddPixel(canvas, ypxl2, xpxl2, (1.0f - yend_f) * xgap);
    AddPixel(canvas, ypxl2 + 1, xpxl2, yend_f * xgap);

    /* Inner part of line. */
    for (x = xpxl1 + 1; x < xpxl2; x++, intery += gradient) {
      float i, f;
      f = modff(intery, &i) * 128.0f;
      AddPixel(canvas, i, x, 128.0f - f);
      AddPixel(canvas, i + 1, x, f);
    }
  } else {
    /* Draw second endpoint. */
    AddPixel(canvas, xpxl2, ypxl2, (1.0f - yend_f) * xgap);
    AddPixel(canvas, xpxl2, ypxl2 + 1, yend_f * xgap);

    /* Inner part of line. */
    for (x = xpxl1 + 1; x < xpxl2; x++, intery += gradient) {
      float i, f;
      f = modff(intery, &i) * 128.0f;
      AddPixel(canvas, x, i, 128.0f - f);
      AddPixel(canvas, x, i + 1, f);
    }
  }
}
