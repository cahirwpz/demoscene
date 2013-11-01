#include "gfx/line.h"

/*
 * Liang-Barsky line clipping algorithm taken from:
 *
 * http://www.skytopia.com/project/articles/compsci/clipping.html
 */

typedef struct {
  int p, q;
} LBEdgeT;

static inline int ext16(int p) {
  asm("swap  %0;"
      "clr.w %0;"
      : "+d" (p));
  return p;
}

static inline int round16(int p) {
  asm("add.l #0x8000,%0;"
      "swap  %0;"
      "ext.l %0;"
      : "+d" (p));
  return p;
}

static bool LiangBarsky(PointT *p1, PointT *p2, PixBufT *canvas) {
  static LBEdgeT edge[4];

  int t0 = 0;
  int t1 = 0x10000;
  int xdelta = p2->x - p1->x;
  int ydelta = p2->y - p1->y;
  int i;

  edge[0].p = -xdelta;
  edge[0].q = p1->x - 0;
  edge[1].p = xdelta;
  edge[1].q = (canvas->width - 1) - p1->x;
  edge[2].p = -ydelta;
  edge[2].q = p1->y - 0;
  edge[3].p = ydelta;
  edge[3].q = (canvas->height - 1) - p1->y;

  for(i = 0; i < 4; i++) {
    int p = edge[i].p;
    int q = edge[i].q;

    if (p == 0) {
      if (q < 0)
        return false;
    } else {
      int r = ext16(q) / p;

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
  }

  p2->x = p1->x + round16(t1 * xdelta);
  p2->y = p1->y + round16(t1 * ydelta);
  p1->x += round16(t0 * xdelta);
  p1->y += round16(t0 * ydelta);

  return true;
}

void DrawLineUnsafe(PixBufT *canvas, int xs, int ys, int xe, int ye) {
  int stride = canvas->width;
  int dx, dy, step, dg, dg1, dg2, db1, db2, n;

  if (ys > ye) {
    swapr(xs, xe);
    swapr(ys, ye);
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
