#include "gfx/line.h"

void DrawLine(CanvasT *canvas, int xs, int ys, int xe, int ye) {
  int stride = GetCanvasWidth(canvas);
  int dx, dy, step, dg, dg1, dg2, db1, db2, n;

  if (ys > ye) {
    swapi(xs, xe);
    swapi(ys, ye);
  }

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

  dx = abs(xe - xs);
  dy = ye - ys;
  step = (xe < xs) ? -1 : 1;

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

  {
    uint8_t *pixels = GetCanvasPixelData(canvas) + ys * stride + xs;
    uint8_t color = canvas->fg_col;

    for (;;) {
      *pixels = color;

      if (!n--)
        break;

      if (dg > 0) {
        pixels += db1;
        dg += dg1;
      } else {
        dg += dg2;
      }

      pixels += db2;
    }
  }
}

void DrawPolyLine(CanvasT *canvas, PointT *points, int n, bool closed) {
  int i;

  for (i = 0; i < (n - 1); i++)
    DrawLine(canvas, points[i].x, points[i].y, points[i+1].x, points[i+1].y);

  if (closed)
    DrawLine(canvas, points[i].x, points[i].y, points[0].x, points[0].y);
}
