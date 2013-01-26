#define NDEBUG
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

typedef struct EdgeScan {
  int height, width;

  int16_t x, dx;
  int16_t xerr, dxerr, nxerr;
  int16_t y;
} EdgeScanT;

static EdgeScanT l12, l13, l23;

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

static void InitEdgeScan(EdgeScanT *e, fixed_t ys, fixed_t ye, fixed_t xs, fixed_t xe) {
  fixed_t height = ye - ys;
  fixed_t width = xe - xs;

  e->height = fx_rint(ye) - fx_rint(ys);
  e->width = fx_rint(xe) - fx_rint(xs);
  e->nxerr = height;

  if (height)
    remquo16(width, height, &e->dx, &e->dxerr);

  /* X value and error. */
  e->x = fx_rint(xs);
  e->xerr = 0;

  {
    fixed_t ys_centered = ys + float_to_fx(0.5f);
    fixed_t ys_prestep = fx_ceil(ys_centered) - ys_centered;

    if (height) {
      int16_t r, q;

      remquo16(fx_mul(width, ys_prestep), height, &q, &r);

      e->x += q;
      e->xerr += r;
    }

    /* Y value. */
    e->y = fx_int(ys_centered);
  }
}

static inline void IterEdgeScan(EdgeScanT *e) {
  e->x += e->dx;
  e->xerr += e->dxerr;

  if (e->xerr >= e->nxerr) {
    e->xerr -= e->nxerr;
    e->x++;
  }

  e->y++;
}

static inline bool CmpEdgeScan(EdgeScanT *e1, EdgeScanT *e2) {
  if (e1->dx == e2->dx)
    return e1->dxerr < e2->dxerr;
  else
    return e1->dx < e2->dx;
}

/* Segment structure & routines. */

typedef struct Segment {
  uint8_t *pixels;
  size_t stride;
  uint8_t color;
} SegmentT;

static SegmentT segment;

static inline void InitSegment(CanvasT *canvas, int y) {
  int stride = GetCanvasWidth(canvas);

  segment.color  = GetCanvasFgCol(canvas);
  segment.stride = stride;
  segment.pixels = GetCanvasPixelData(canvas) + y * stride;
}

__regargs static void DrawTriangleSegment(EdgeScanT *left, EdgeScanT *right,
                                          int h)
{
  while (h-- > 0) {
    int x = left->x;
    int w = right->x - left->x;

    uint8_t *pixels = segment.pixels + x;
    uint8_t color = segment.color;

    LOG("Line: (%d, %d..%d)", left->y, left->x, right->x);

    do {
      *pixels++ = color;
    } while (--w >= 0);

    segment.pixels += segment.stride;
    IterEdgeScan(left);
    IterEdgeScan(right);
  }
}

/* Triangle rasterization routine. */

void DrawTriangle(CanvasT *canvas, float x1f, float y1f, float x2f, float y2f,
                  float x3f, float y3f)
{
  {
    fixed_t x1 = float_to_fx(x1f);
    fixed_t y1 = float_to_fx(y1f);
    fixed_t x2 = float_to_fx(x2f);
    fixed_t y2 = float_to_fx(y2f);
    fixed_t x3 = float_to_fx(x3f);
    fixed_t y3 = float_to_fx(y3f);

    if (y1 > y2) {
      swapr(x1, x2);
      swapr(y1, y2);
    }

    if (y1 > y3) {
      swapr(x1, x3);
      swapr(y1, y3);
    }

    if (y2 > y3) {
      swapr(x2, x3);
      swapr(y2, y3);
    }

    LOG("Triangle: (%f, %f) (%f, %f) (%f, %f).", x1f, y1f, x2f, y2f, x3f, y3f);

    InitEdgeScan(&l12, y1, y2, x1, x2);
    InitEdgeScan(&l13, y1, y3, x1, x3);
    InitEdgeScan(&l23, y2, y3, x2, x3);
  }

  InitSegment(canvas, l12.y);

  {
    bool longOnRight;

    if (l12.height == 0)
      longOnRight = (l12.width < 0);
    else if (l23.height == 0)
      longOnRight = (l23.width > 0);
    else
      longOnRight = CmpEdgeScan(&l12, &l13);

    {
      EdgeScanT *left  = longOnRight ? &l12 : &l13;
      EdgeScanT *right = longOnRight ? &l13 : &l12;

      DrawTriangleSegment(left, right, l12.height);

      if (longOnRight)
        left = &l23;
      else
        right = &l23;

      DrawTriangleSegment(left, right, l23.height);
    }
  }
}
