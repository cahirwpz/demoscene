#include "gfx/triangle.h"
#include "system/debug.h"

typedef struct EdgeScan {
  float d, v;
} EdgeScanT;

typedef struct Segment {
  uint8_t *pixels;
  size_t stride;
  uint8_t color;
} SegmentT;

static void DrawTriangleSegment(EdgeScanT *xs, EdgeScanT *xe, SegmentT *seg,
                                int y1, int y2) {
  LOG("Line: (%ld, %ld..%ld)\n", y1, (int)xs->v, (int)xe->v);

  bool same = ((int)xs->v == (int)xe->v);

  for (; y1 <= y2; y1++) {
    int x1 = xs->v;
    int x2 = xe->v;

    uint8_t *pixels = seg->pixels + x1;

    for (; x1 <= x2; x1++)
      *pixels++ = seg->color;

    seg->pixels += seg->stride;
    xs->v += xs->d;
    xe->v += xe->d;
  }

  if (!same && abs((int)xs->v - (int)xe->v) > 1)
    LOG("Line: (%ld, %ld..%ld)\n", y2, (int)xs->v, (int)xe->v);
}

void DrawTriangle(CanvasT *canvas,
                  int x1, int y1, int x2, int y2, int x3, int y3) {
  if (y1 > y2) {
    swapi(x1, x2);
    swapi(y1, y2);
  }

  if (y1 > y3) {
    swapi(x1, x3);
    swapi(y1, y3);
  }

  if (y2 > y3) {
    swapi(x2, x3);
    swapi(y2, y3);
  }

  /*
  LOG("Triangle: (%ld, %ld) (%ld, %ld) (%ld, %ld)\n",
      (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3); */

  if (x1 == x2 && x2 == x3) {
    LOG("Triangle is too thin.\n");
    return;
  }

  SegmentT seg;

  seg.color = GetCanvasFgCol(canvas);
  seg.stride = GetCanvasWidth(canvas);
  seg.pixels = GetCanvasPixelData(canvas) + y1 * seg.stride;

  float dx12 = x2 - x1;
  float dx13 = x3 - x1;
  float dx23 = x3 - x2;
  float dy12 = y2 - y1;
  float dy13 = y3 - y1;
  float dy23 = y3 - y2;

  if (y1 == y2) {
    EdgeScanT l13 = { dx13 / dy13, x1 };
    EdgeScanT l23 = { dx23 / dy23, x2 };

    if (x1 < x2)
      DrawTriangleSegment(&l13, &l23, &seg, y2, y3);
    else
      DrawTriangleSegment(&l23, &l13, &seg, y2, y3);
  }
  else if (y2 == y3)
  {
    EdgeScanT l12 = { dx12 / dy12, x1 };
    EdgeScanT l13 = { dx13 / dy13, x1 };

    if (x2 < x3)
      DrawTriangleSegment(&l12, &l13, &seg, y1, y2);
    else
      DrawTriangleSegment(&l13, &l12, &seg, y1, y2);
  }
  else
  {
    EdgeScanT l12 = { dx12 / dy12, x1 };
    EdgeScanT l13 = { dx13 / dy13, x1 };
    EdgeScanT l23 = { dx23 / dy23, x2 };

    if (l12.d < l13.d) {
      DrawTriangleSegment(&l12, &l13, &seg, y1, y2);
      DrawTriangleSegment(&l23, &l13, &seg, y2 + 1, y3);
    } else {
      DrawTriangleSegment(&l13, &l12, &seg, y1, y2);
      DrawTriangleSegment(&l13, &l23, &seg, y2 + 1, y3);
    }
  }
}
