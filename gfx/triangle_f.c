#ifndef NDEBUG
#define NDEBUG
#endif

#include "gfx/triangle.h"
#include "std/debug.h"
#include "std/math.h"

/* EdgeScan structure & routines. */

typedef struct EdgeScan {
  int height, width, y;
  float x, dx;
} EdgeScanT;

__regargs static void InitEdgeScan(EdgeScanT *e, TriPoint *ps, TriPoint *pe) {
  float xs = ps->x;
  float ys = ps->y;
  float xe = pe->x;
  float ye = pe->y;

  float height = ye - ys;
  float width = xe - xs;

  e->height = lroundf(ye) - lroundf(ys);
  e->width = lroundf(xe) - lroundf(xs);

  if (height)
    e->dx = width / height;

  e->x = xs;
  e->y = lroundf(ys);

  if (height) {
    float ys_centered = ys + 0.5f;
    float ys_prestep = ceil(ys_centered) - ys_centered;

    e->x += e->dx * ys_prestep;
  }
}

static inline void IterEdgeScan(EdgeScanT *e) {
  e->x += e->dx;
  e->y++;
}

static inline bool CmpEdgeScan(EdgeScanT *e1, EdgeScanT *e2) {
  return e1->dx < e2->dx;
}

/* Segment routines. */
static inline void 
DrawTriangleSpan(uint8_t *pixels, const uint8_t color,
                 int xs, int xe, int y)
{
  int n;

  n = xe - xs;
  pixels += xs;

  LOG("Line: (%d, %d..%d)", y, xs, xe);

  do {
    *pixels++ = color;
  } while (--n >= 0);
}

__attribute__((regparm(4))) static void
DrawTriangleSegment(PixBufT *canvas, EdgeScanT *left, EdgeScanT *right,
                    int ys, int h)
{
  uint8_t *pixels = canvas->data + ys * canvas->width;
  const uint8_t color = canvas->fgColor;
  int width = canvas->width;
  int ye = ys + h;

  for (; ys < ye; ys++) {
    DrawTriangleSpan(pixels, color, lroundf(left->x), lroundf(right->x), ys);

    pixels += width;

    IterEdgeScan(left);
    IterEdgeScan(right);
  }
}

/* Triangle rasterization routine. */
void DrawTriangle(PixBufT *canvas,
                  TriPoint *p1, TriPoint *p2, TriPoint *p3)
{
  EdgeScanT l12, l13, l23;
  bool longOnRight;

  if (p1->y > p2->y)
    swapr(p1, p2);

  if (p1->y > p3->y)
    swapr(p1, p3);

  if (p2->y > p3->y)
    swapr(p2, p3);

  LOG("Triangle: (%f, %f) (%f, %f) (%f, %f).",
      p1->x, p1->y, p2->x, p2->y, p3->x, p3->y);

  InitEdgeScan(&l12, p1, p2);
  InitEdgeScan(&l13, p1, p3);
  InitEdgeScan(&l23, p2, p3);

  if (l12.height == 0)
    longOnRight = (l12.width < 0);
  else if (l23.height == 0)
    longOnRight = (l23.width > 0);
  else
    longOnRight = CmpEdgeScan(&l12, &l13);

  {
    EdgeScanT *left  = longOnRight ? &l12 : &l13;
    EdgeScanT *right = longOnRight ? &l13 : &l12;

    DrawTriangleSegment(canvas, left, right, p1->y, l12.height);

    if (longOnRight)
      left = &l23;
    else
      right = &l23;

    DrawTriangleSegment(canvas, left, right, p2->y, l23.height);
  }
}
