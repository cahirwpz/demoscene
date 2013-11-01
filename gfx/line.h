#ifndef __GFX_LINE_H__
#define __GFX_LINE_H__

#include "gfx/common.h"
#include "gfx/pixbuf.h"

void DrawLineUnsafe(PixBufT *canvas, int x1, int y1, int x2, int y2);
void DrawLine(PixBufT *canvas, int x1, int y1, int x2, int y2);
void DrawPolyLine(PixBufT *canvas, PointT *points, int n, bool closed);

#endif
