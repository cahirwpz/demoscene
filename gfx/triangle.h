#ifndef __GFX_TRIANGLE_H__
#define __GFX_TRIANGLE_H__

#include "gfx/pixbuf.h"

typedef struct {
  float x, y;
} TriPoint;

void DrawTriangle(PixBufT *canvas, TriPoint *p1, TriPoint *p2, TriPoint *p3);

#endif
