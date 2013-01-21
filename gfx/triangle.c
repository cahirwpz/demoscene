#define NDEBUG
#include "gfx/triangle.h"
#include "std/debug.h"
#include "std/math.h"

typedef struct EdgeScan {
  float d, v;
} EdgeScanT;

static EdgeScanT l12, l13, l23;

static inline void InitEdgeScan(EdgeScanT *e, int dx, int dy, int x) {
  if (dy) 
    e->d = (float)dx / (float)dy;
  e->v = x;
}

typedef struct Segment {
  uint8_t *pixels;
  size_t stride;
  uint8_t color;
} SegmentT;

static SegmentT segment;

__regargs static void DrawTriangleSegment(EdgeScanT *xs, EdgeScanT *xe,
                                          int y, int h) {
  bool same = ((int)xs->v == (int)xe->v);

  LOG("Line: (%d, %f..%f)", y, xs->v, xe->v);

  do {
    int x = lroundf(xs->v);
    int w = lroundf(xe->v - xs->v);

    uint8_t *pixels = segment.pixels + x;
    uint8_t color = segment.color;

    do {
      *pixels++ = color;
    } while (--w >= 0);

    segment.pixels += segment.stride;
    xs->v += xs->d;
    xe->v += xe->d;
    y++;
  } while (--h > 0);

  if (!same && abs((int)xs->v - (int)xe->v) > 1)
    LOG("Line: (%d, %f..%f)", y, xs->v, xe->v);
}

void DrawTriangle(CanvasT *canvas,
                  int x1, int y1, int x2, int y2, int x3, int y3) {
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

  segment.color  = GetCanvasFgCol(canvas);
  segment.stride = GetCanvasWidth(canvas);
  segment.pixels = GetCanvasPixelData(canvas) + y1 * segment.stride;

  LOG("Triangle: (%d, %d) (%d, %d) (%d, %d).", x1, y1, x2, y2, x3, y3);

  {
    bool longOnRight;
    int  topHeight;
    int  bottomHeight;

    {
      int dx12 = x2 - x1;
      int dx13 = x3 - x1;
      int dx23 = x3 - x2;
      int dy12 = y2 - y1;
      int dy13 = y3 - y1;
      int dy23 = y3 - y2;

      InitEdgeScan(&l12, dx12, dy12, x1);
      InitEdgeScan(&l13, dx13, dy13, x1);
      InitEdgeScan(&l23, dx23, dy23, x2);

      topHeight = dy12;
      bottomHeight = dy23;

      if (topHeight == 0)
        longOnRight = (dx12 < 0);
      else if (bottomHeight == 0)
        longOnRight = (dx23 > 0);
      else
        longOnRight = (l12.d < l13.d);
    }

    {
      EdgeScanT *left  = longOnRight ? &l12 : &l13;
      EdgeScanT *right = longOnRight ? &l13 : &l12;

      if (topHeight)
        DrawTriangleSegment(left, right, y1, topHeight);

      if (longOnRight)
        left = &l23;
      else
        right = &l23;

      if (bottomHeight)
        DrawTriangleSegment(left, right, y2, bottomHeight);
    }
  }
}
