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

#define dx xe
#define dy ye
__attribute__((regparm(4))) void
DrawLineUnsafe(PixBufT *canvas, int xs, int ys, int xe, int ye) {
  uint8_t *pixels;
  uint8_t color;
  int step, stride;

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

  if (xs < xe) {
    dx = xe - xs;
    step = 1;
  } else {
    dx = xs - xe;
    step = -1;
  }

  dy = ye - ys;

  color = canvas->fgColor;
  stride = canvas->width;
  pixels = canvas->data + ys * stride + xs;

  /* (xs, ys, xe, ye) unused from now on */

  if (dx < dy) {
    int dg2 = dx * 2;
    int dg = dg2 - dy;
    int dg1 = dg - dy;

    do {
      *pixels = color;

      if (dg > 0) {
        pixels += step;
        dg += dg1;
      } else {
        dg += dg2;
      }

      pixels += stride;
    } while (--dy > 0);
  } else {
    int dg2 = dy * 2;
    int dg = dg2 - dx;
    int dg1 = dg - dx;

    do {
      *pixels = color;

      if (dg > 0) {
        pixels += stride;
        dg += dg1;
      } else {
        dg += dg2;
      }

      pixels += step;
    } while (--dx > 0);
  }
}
#undef dx
#undef dy

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
