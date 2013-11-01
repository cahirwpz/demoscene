#include "gfx/line.h"

/*
 * Liang-Barsky line clipping algorithm taken from:
 *
 * http://www.skytopia.com/project/articles/compsci/clipping.html
 */

static bool LiangBarsky(PointT *p1, PointT *p2, PixBufT *canvas) {
  float t0 = 0.0;
  float t1 = 1.0;
  int xdelta = p2->x - p1->x;
  int ydelta = p2->y - p1->y;
  int edge;

  for(edge = 0; edge < 4; edge++) {
    int p = 0, q = 0;
    float r;

    switch (edge) {
      case 0:
        p = -xdelta;
        q = p1->x; // - minX
        break;

      case 1:
        p = xdelta;
        q = (canvas->width - 1) - p1->x;
        break;

      case 2:
        p = -ydelta;
        q = p1->y; // - minY
        break;

      case 3:
        p = ydelta;
        q = (canvas->height - 1) - p1->y;
        break;
    }

    if (p == 0 && q < 0)
      return false;

    if (p == 0)
      continue;

    r = (float)q / (float)p;

    if (p < 0) {
      if (r > t1)
        return false;

      if (r > t0)
        t0 = r;
    } else {
      if (r < t0)
        return false;

      if (r < t1)
        t1 = r;
    }
  }

  if (t1 < 1.0f) {
    p2->x = p1->x + t1 * (float)xdelta;
    p2->y = p1->y + t1 * (float)ydelta;
  }

  if (t0 > 0.0f) {
    p1->x += t0 * (float)xdelta;
    p1->y += t0 * (float)ydelta;
  }

  return true;
}

void DrawLineUnsafe(PixBufT *canvas, int xs, int ys, int xe, int ye) {
  int stride = canvas->width;
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
    uint8_t *pixels = canvas->data + ys * stride + xs;
    uint8_t color = canvas->fgColor;

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

void DrawLine(PixBufT *canvas, int xs, int ys, int xe, int ye) {
  PointT p1 = { xs, ys };
  PointT p2 = { xe, ye };

  if (LiangBarsky(&p1, &p2, canvas))
    DrawLineUnsafe(canvas, p1.x, p1.y, p2.x, p2.y);
}

void DrawPolyLine(PixBufT *canvas, PointT *points, int n, bool closed) {
  int i;

  for (i = 0; i < (n - 1); i++)
    DrawLine(canvas, points[i].x, points[i].y, points[i+1].x, points[i+1].y);

  if (closed)
    DrawLine(canvas, points[i].x, points[i].y, points[0].x, points[0].y);
}
