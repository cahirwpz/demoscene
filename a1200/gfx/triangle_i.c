#ifndef NDEBUG
#define NDEBUG
#endif

#include "gfx/triangle.h"
#include "std/debug.h"
#include "std/math.h"

static inline void remquo16(int32_t n, int16_t d, int16_t *quo, int16_t *rem) {
  int16_t r;

  asm("divsw %2,%0\n\t"
      "movel %0,%1\n\t"
      "swap  %1\n\t"
      : "+d" (n), "=&d" (r)
      : "dm" (d));

  /* Remainder is not supposed to be negative so fix it. */
  if (r < 0) {
    r += d;
    n--;
  }

  *rem = r;
  *quo = n;
}

typedef int32_t fixed_t;

static inline fixed_t float_to_fx(float v) {
  return v * 16.0f;
}

static inline fixed_t fx_round(fixed_t v) {
  return (v + 8) & ~15;
}

static inline fixed_t fx_ceil(fixed_t v) {
  return (v + 15) & ~15;
}

static inline int fx_rint(fixed_t v) {
  return (v + 8) >> 4;
}

static inline int fx_int(fixed_t v) {
  return v >> 4;
}

static inline fixed_t fx_mul(fixed_t u, fixed_t v) {
  return (u * v) >> 4;
} 

/* EdgeScan structure & routines. */

typedef struct {
  fixed_t x, y;
} _TriPoint;

typedef struct EdgeScan {
  int height, width;

  int16_t x, dx;
  int16_t xerr, dxerr, nxerr;
} EdgeScanT;

/*
 * 1) DDA equation:
 *
 *       x_1 - x_0
 *   x = --------- * (y - y_0) + x_0
 *       y_1 - y_0
 *
 *   where x_0, x_1, y_0, y_1 are integers.
 *
 * 2) Ceil and floor equations:
 *
 *   a   | a |   a mod b
 *   - = | - | + -------
 *   b   + b +      b
 *
 *   + a +   | a - 1 |       | a + b - 1 |
 *   | - | = | ----- | + 1 = | --------- |
 *   | b |   +   b   +       +     b     +
 */

__attribute__((regparm(4))) static void
InitEdgeScan(EdgeScanT *e, _TriPoint *ps, _TriPoint *pe) {
  fixed_t xs = ps->x;
  fixed_t ys = ps->y;
  fixed_t xe = pe->x;
  fixed_t ye = pe->y;

  fixed_t height = ye - ys;
  fixed_t width = xe - xs;

  e->x = fx_rint(xs);
  e->width = fx_rint(xe) - fx_rint(xs);
  e->height = fx_rint(ye) - fx_rint(ys);
  e->xerr = 0;
  e->nxerr = height;

  if (height)
    remquo16(width, height, &e->dx, &e->dxerr);

  {
    fixed_t ys_centered = ys + float_to_fx(0.5f);
    fixed_t ys_prestep = fx_ceil(ys_centered) - ys_centered;

    if (height) {
      int16_t q;

      remquo16(fx_mul(width, ys_prestep), height, &q, &e->xerr);

      e->x += q;
    }
  }
}

static inline void IterEdgeScan(EdgeScanT *e) {
  e->x += e->dx;
  e->xerr += e->dxerr;

  if (e->xerr >= e->nxerr) {
    e->xerr -= e->nxerr;
    e->x++;
  }
}

static inline bool CmpEdgeScan(EdgeScanT *e1, EdgeScanT *e2) {
  if (e1->dx == e2->dx)
    return e1->dxerr < e2->dxerr;
  else
    return e1->dx < e2->dx;
}

/* Segment routines. */
static inline void 
DrawTriangleSpan(uint8_t *pixels, const uint8_t color,
                 int xs, int xe, const int max_x)
{
  int n;

  if (xs < 0)
    xs = 0;

  if (xe > max_x)
    xe = max_x;

  n = xe - xs;
  pixels += xs;

  LOG("Line: (%d..%d)", xs, xe);

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
  int height = canvas->height;
  int ye = ys + h;

  for (; ys < ye; ys++) {
    if (ys >= 0 && ys < height && left->x < width && right->x >= 0)
      DrawTriangleSpan(pixels, color, left->x, right->x, width - 1);

    pixels += width;

    IterEdgeScan(left);
    IterEdgeScan(right);
  }
}

/* Triangle rasterization routine. */
void DrawTriangle(PixBufT *canvas,
                  TriPoint *p1f, TriPoint *p2f, TriPoint *p3f)
{
  EdgeScanT l12, l13, l23;
  bool longOnRight;

  /* Vertices translation could be dropped at some point... */
  _TriPoint points[] = {
    { float_to_fx(p1f->x), float_to_fx(p1f->y) },
    { float_to_fx(p2f->x), float_to_fx(p2f->y) },
    { float_to_fx(p3f->x), float_to_fx(p3f->y) }
  };

  _TriPoint *p1 = &points[0];
  _TriPoint *p2 = &points[1];
  _TriPoint *p3 = &points[2];

  if (p1->y > p2->y)
    swapr(p1, p2);

  if (p1->y > p3->y)
    swapr(p1, p3);

  if (p2->y > p3->y)
    swapr(p2, p3);

  LOG("Triangle: (%f, %f) (%f, %f) (%f, %f).",
      p1f->x, p1f->y, p2f->x, p2f->y, p3f->x, p3f->y);

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

    DrawTriangleSegment(canvas, left, right, fx_rint(p1->y), l12.height);

    if (longOnRight)
      left = &l23;
    else
      right = &l23;

    DrawTriangleSegment(canvas, left, right, fx_rint(p2->y), l23.height);
  }
}
