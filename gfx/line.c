#include "gfx/line.h"

void draw_line(canvas_t *canvas, int xs, int ys, int xe, int ye) {
  int stride = canvas->bitmap->width;

  if (ys > ye) {
    swapi(xs, xe);
    swapi(ys, ye);
  }

  uint8_t *bitmap = canvas->bitmap->data + ys * stride + xs;

  int dx = abs(xe - xs);
  int dy = ye - ys;

  /*  quarters:
   *
   *  \ 1 | 3 /
   *   \  |  /
   *    \ | /
   *  0  \|/  2
   * -----x-----
   *  2  /|\  0
   *    / | \
   *   /  |  \
   *  / 3 | 1 \
   *
   */

  int step = (xe < xs) ? -1 : 1;

  int dg, dg1, dg2, db1, db2, n;

  if (dx < dy) {
    dg1 = (dx - dy) << 1;
    dg2 = (dx << 1);
    dg  = dg2 - dy;

    db1 = step;
    db2 = stride;

    n = dy;
  } else {
    dg1 = (dy - dx) << 1;
    dg2 = (dy << 1);
    dg  = dg2 - dx;

    db1 = stride;
    db2 = step;

    n = dx;
  }

  uint8_t color = canvas->fg_col;

  for (;;) {
    *bitmap = color;

    if (!n--)
      break;

    if (dg > 0) {
      bitmap += db1;
      dg += dg1;
    } else {
      dg += dg2;
    }

    bitmap += db2;
  }
}

void draw_poly_line(canvas_t *canvas, point_t *points, int n, bool closed) {
  int i;

  for (i = 0; i < (n - 1); i++)
    draw_line(canvas, points[i].x, points[i].y, points[i+1].x, points[i+1].y);

  if (closed)
    draw_line(canvas, points[i].x, points[i].y, points[0].x, points[0].y);
}
