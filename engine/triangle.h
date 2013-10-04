#ifndef __ENGINE_TRIANGLE_H__
#define __ENGINE_TRIANGLE_H__

#include "gfx/pixbuf.h"
#include "std/fp16.h"

typedef struct EdgeScan {
  int xs;
  int xe;
  int ys;
  int ye;

  bool visible;

  FP16 x;
  FP16 dx;
} EdgeScanT;

__attribute__((regparm(4))) void
InitEdgeScan(EdgeScanT *e, FP16 ys, FP16 ye, FP16 xs, FP16 xe);

void RasterizeTriangle(PixBufT *canvas,
                       EdgeScanT *e1, EdgeScanT *e2, EdgeScanT *e3);

#endif
