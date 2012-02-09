#include "gfx/triangle.h"
#include "system/debug.h"

typedef struct Segment {
  uint8_t *pixels;
  size_t stride;

  float xs, dxs;
  float xe, dxe;

  uint8_t color;
} SegmentT;

static void DrawTriangleSegment(SegmentT *seg, int y1, int y2) {
  //LOG("Line: (%ld, %ld..%ld)\n", y1, (int)seg->xs, (int)seg->xe);

  bool same = ((int)seg->xs == (int)seg->xe);

  for (; y1 <= y2; y1++) {
    int x1 = (int)seg->xs;
    int x2 = (int)seg->xe;

    uint8_t *pixels = seg->pixels + x1;

    for (; x1 <= x2; x1++)
      *pixels++ = seg->color;

    seg->pixels += seg->stride;
    seg->xs += seg->dxs;
    seg->xe += seg->dxe;
  }

  if (!same && abs((int)seg->xs - (int)seg->xe) > 1)
    LOG("Line: (%ld, %ld..%ld)\n", y2, (int)seg->xs, (int)seg->xe);
}

void DrawTriangle(CanvasT *canvas,
                  float x1, float y1, float x2, float y2, float x3, float y3) {
  if (y1 > y2) {
    swapf(x1, x2);
    swapf(y1, y2);
  }

  if (y1 > y3) {
    swapf(x1, x3);
    swapf(y1, y3);
  }

  if (y2 > y3) {
    swapf(x2, x3);
    swapf(y2, y3);
  }

  /*
  LOG("Triangle: (%ld, %ld) (%ld, %ld) (%ld, %ld)\n",
      (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3); */

  if ((int)x1 == (int)x2 && (int)x2 == (int)x3) {
    LOG("Triangle is too thin.\n");
    return;
  }

  int y1i = (int)(y1 + 0.5f);
  int y2i = (int)(y2 + 0.5f);
  int y3i = (int)(y3 + 0.5f);

  SegmentT seg;

  seg.color = GetCanvasFgCol(canvas);
  seg.stride = GetCanvasWidth(canvas);
  seg.pixels = GetCanvasPixelData(canvas) + y1i * seg.stride;

  float dx12 = x2 - x1;
  float dx13 = x3 - x1;
  float dx23 = x3 - x2;
  float dy12 = y2 - y1;
  float dy13 = y3 - y1;
  float dy23 = y3 - y2;

  float y1e = y1 - y1i;
  float y2e = y2 - y2i;

  float d13 = dx13 / dy13;

  if (y1i == y2i) {
    float d23 = dx23 / dy23;

    seg.xs = x1 + d13 * y1e;
    seg.xe = x2 + d23 * y2e;
    seg.dxs = d13;
    seg.dxe = d23;

    if (x1 > x2) {
      swapf(seg.dxs, seg.dxe);
      swapf(seg.xs, seg.xe);
    }

    DrawTriangleSegment(&seg, y2i, y3i);
  }
  else if (y2i == y3i)
  {
    float d12 = dx12 / dy12;

    seg.xs = x1 + d12 * y1e;
    seg.xe = x1 + d13 * y1e;
    seg.dxs = d12;
    seg.dxe = d13; 

    if (x2 > x3) {
      swapf(seg.dxs, seg.dxe);
      swapf(seg.xs, seg.xe);
    }

    DrawTriangleSegment(&seg, y1i, y2i);
  }
  else
  {
    float d12 = dx12 / dy12;
    float d23 = dx23 / dy23;

    seg.xs = x1 + d12 * y1e;
    seg.xe = x1 + d13 * y1e;
    seg.dxs = d12;
    seg.dxe = d13;

    if (d12 > d13) {
      swapf(seg.dxs, seg.dxe);
      swapf(seg.xs, seg.xe);
    }

    DrawTriangleSegment(&seg, y1i, y2i);

    if (d12 > d13) {
      seg.xe = x2 + d23 * y2e;
      seg.dxe = d23;
    } else {
      seg.xs = x2 + d23 * y2e;
      seg.dxs = d23;
    }

    DrawTriangleSegment(&seg, y2i+1, y3i);
  }
}
