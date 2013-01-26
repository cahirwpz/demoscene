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

/* EdgeScan structure & routines. */

typedef struct EdgeScan {
  int height, width;

  int16_t x, dx;
  int16_t xerr, dxerr;
  int16_t y;
} EdgeScanT;

static EdgeScanT l12, l13, l23;

static inline void InitEdgeScan(EdgeScanT *e, int ys, int ye, int xs, int xe) {
  int height = ye - ys;
  int width = xe - xs;

  /* Divisor, divident, remainder and quotient. */
  e->height = height;
  e->width = width;

  if (height)
    remquo16(width, height, &e->dx, &e->dxerr);

  /* X value and error. */
  e->x = xs;
  e->xerr = 0;

  /* Y value. */
  e->y = ys;
}

static inline void IterEdgeScan(EdgeScanT *e) {
  e->x += e->dx;
  e->xerr += e->dxerr;

  if (e->xerr >= e->height) {
    e->xerr -= e->height;
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

    LOG("Line: (%d, %f..%f)", left->y, left->x, right->x);

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
    int x1 = lroundf(x1f);
    int y1 = lroundf(y1f);
    int x2 = lroundf(x2f);
    int y2 = lroundf(y2f);
    int x3 = lroundf(x3f);
    int y3 = lroundf(y3f);

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

    LOG("Triangle: (%d, %d) (%d, %d) (%d, %d).", x1, y1, x2, y2, x3, y3);

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
