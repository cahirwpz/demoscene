#ifndef __ENGINE_TRIANGLE_H__
#define __ENGINE_TRIANGLE_H__

#include "gfx/pixbuf.h"
#include "std/fp16.h"

typedef struct EdgeScan {
  int16_t xs;
  int16_t xe;
  int16_t ys;
  int16_t ye;

  FP16 x;
  FP16 dx;
} EdgeScanT;

__attribute__((regparm(4))) void
InitEdgeScan(EdgeScanT *e, float ys, float ye, float xs, float xe);

void RasterizeTriangle(PixBufT *canvas,
                       EdgeScanT *e1, EdgeScanT *e2, EdgeScanT *e3);

#endif
