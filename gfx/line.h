#ifndef __GFX_LINE_H__
#define __GFX_LINE_H__

#include "gfx/canvas.h"

void DrawLine(CanvasT *canvas, int x1, int y1, int x2, int y2);
void DrawPolyLine(CanvasT *canvas, PointT *points, int n, bool closed);

#endif
