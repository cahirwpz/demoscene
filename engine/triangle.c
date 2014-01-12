#include "engine/triangle.h"
#include "std/debug.h"
#include "std/math.h"

__attribute__((regparm(4))) void
InitEdgeScan(EdgeScanT *e, float ys, float ye, float xs, float xe) {
  float height = ye - ys;
  float width = xe - xs;

  e->xs = lroundf(xs);
  e->xe = lroundf(xe);
  e->ys = lroundf(ys);
  e->ye = lroundf(ye);

  if (e->ys < e->ye) {
    float dx = width / height;
    float ys_centered = ys + 0.5f;
    float ys_prestep = ceil(ys_centered) - ys_centered;
    float x = xs + ys_prestep * dx;

    e->x = FP16_float(x);
    e->dx = FP16_float(dx);
  }

  e->done = false;
}

/* Segment routines. */
__attribute__((regparm(4))) static void
RasterizeTriangleSegment(PixBufT *canvas, EdgeScanT *left, EdgeScanT *right,
                         int ys, int ye)
{
  uint8_t *pixels = canvas->data + ys * canvas->width;
  register const uint8_t color = canvas->fgColor;
  register FP16 lx = left->x;
  register FP16 rx = right->x;
  register FP16 ldx = left->dx;
  register FP16 rdx = right->dx;

  while (ys < ye) {
    int16_t xs = FP16_rintf(lx);
    int16_t xe = FP16_rintf(rx);
    register uint8_t *span = pixels + xs;
    register int16_t n = xe - xs;

    do {
      *span++ = color;
    } while (--n > 0);

    pixels += canvas->width;

    lx = FP16_add(lx, ldx);
    rx = FP16_add(rx, rdx);

    ys++;
  }

  left->x = lx;
  right->x = rx;
}

void RasterizeTriangle(PixBufT *canvas,
                       EdgeScanT *e1, EdgeScanT *e2, EdgeScanT *e3)
{
  if (e1->ys > e2->ys)
    swapr(e1, e2);
  if (e2->ys > e3->ys)
    swapr(e2, e3);
  if (e1->ye > e2->ye)
    swapr(e1, e2);
  if (e1->ye > e3->ye)
    swapr(e1, e3);

  {
    EdgeScanT l12 = *e1;
    EdgeScanT l13 = *e2; /* long one */
    EdgeScanT l23 = *e3;
    EdgeScanT *left;
    EdgeScanT *right;
    bool longOnRight;


    if (l12.ys == l12.ye)
      longOnRight = (l13.xs > l23.xs);
    else if (l23.ys == l23.ye)
      longOnRight = (l13.xe > l12.xe);
    else
      longOnRight = (l12.dx.v < l13.dx.v);

    if (longOnRight) {
      left = &l12; right = &l13;
    } else {
      left = &l13; right = &l12;
    }

    if (l12.ys != l12.ye) {
#if 0
      if (FP16_i(left->x) > FP16_i(right->x) + 3) {
        LOG("l12: (%d, %d) (%d, %d)", l12.xs, l12.ys, l12.xe, l12.ye);
        LOG("l13: (%d, %d) (%d, %d)", l13.xs, l13.ys, l13.xe, l13.ye);
        LOG("l23: (%d, %d) (%d, %d)", l23.xs, l23.ys, l23.xe, l23.ye);
        LOG("long on %s", longOnRight ? "right" : "left");
        LOG("top: xs = %d, xe = %d", FP16_i(left->x), FP16_i(right->x));
      }
#endif
      RasterizeTriangleSegment(canvas, left, right, l12.ys, l12.ye);
    }

    if (longOnRight) {
      left = &l23;
    } else {
      right = &l23;
    }

    if (l23.ys != l23.ye) {
#if 0
      ASSERT(FP16_i(left->x) <= FP16_i(right->x) + 1, "bottom: xs = %d, xe = %d", FP16_i(left->x), FP16_i(right->x));
#endif
      RasterizeTriangleSegment(canvas, left, right, l23.ys, l23.ye);
    }
  }
}
