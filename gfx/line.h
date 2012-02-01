#ifndef __GFX_LINE_H__
#define __GFX_LINE_H__

#include "gfx/canvas.h"

void draw_line(canvas_t *canvas, int x1, int y1, int x2, int y2);
void draw_poly_line(canvas_t *canvas, point_t *points, int n, bool closed);

#endif
