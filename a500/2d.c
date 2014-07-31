#include "2d.h"

__regargs void Identity2D(View2D *view) {
  view->m00 = 1 << 8;
  view->m01 = 0;
  view->x = 0;

  view->m11 = 1 << 8;
  view->m10 = 0;
  view->y = 0;
}

__regargs void Translate2D(View2D *view, WORD x, WORD y) {
  view->x += x;
  view->y += y;
}

__regargs void Scale2D(View2D *view, WORD sx, WORD sy) {
  view->m00 = (view->m00 * sx) >> 8;
  view->m01 = (view->m01 * sy) >> 8;
  view->m10 = (view->m10 * sx) >> 8;
  view->m11 = (view->m11 * sy) >> 8;
}

__regargs void Rotate2D(View2D *view, WORD a) {
  WORD sin = sincos[a & 0x1ff].sin;
  WORD cos = sincos[a & 0x1ff].cos;

  WORD m00 = (LONG)(view->m00 * cos - view->m01 * sin) >> 8;
  WORD m01 = (LONG)(view->m00 * sin + view->m01 * cos) >> 8;
  WORD m10 = (LONG)(view->m10 * cos - view->m11 * sin) >> 8;
  WORD m11 = (LONG)(view->m10 * sin + view->m11 * cos) >> 8;

  view->m00 = m00;
  view->m01 = m01;
  view->m10 = m10;
  view->m11 = m11;
}

__regargs void Transform2D(View2D *view, PointT *out, PointT *in, UWORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;

  while (n--) {
    register WORD x = *src++;
    register WORD y = *src++;

    *dst++ = ((view->m00 * x + view->m01 * y) >> 8) + view->x;
    *dst++ = ((view->m10 * x + view->m11 * y) >> 8) + view->y;
  }
}

__regargs void PointsInsideBox(PointT *in, UBYTE *flags, UWORD n, BoxT *box) {
  WORD *src = (WORD *)in;

  while (n--) {
    WORD x = *src++;
    WORD y = *src++;
    UBYTE f = 0;

    if (x < box->minX)
      f |= PF_LEFT;
    if (x >= box->maxX)
      f |= PF_RIGHT;

    if (y < box->minY)
      f |= PF_TOP;
    if (y >= box->maxY)
      f |= PF_BOTTOM;

    *flags++ = f;
  }
}

#define BITS 8
#define ONE (1 << BITS)
#define HALF (1 << (BITS - 1))

typedef struct {
  WORD p, q1, q2;
} LBEdgeT;

static LBEdgeT edge[2];

/* Liang-Brasky algorithm. */
__regargs BOOL ClipLine(LineT *line, BoxT *box) {
  WORD t0 = 0;
  WORD t1 = ONE;
  WORD xdelta = line->x2 - line->x1;
  WORD ydelta = line->y2 - line->y1;
  WORD i;

  edge[0].p  = -xdelta;
  edge[0].q1 = line->x1 - box->minX;
  edge[0].q2 = box->maxX - line->x1;
  edge[1].p  = -ydelta;
  edge[1].q1 = line->y1 - box->minY;
  edge[1].q2 = box->maxY - line->y1;

  for (i = 0; i < 2; i++) {
    WORD p = edge[i].p;
    WORD r;

    if (p == 0)
      continue;

    if (p < 0) {
      r = div16(edge[i].q1 << BITS, p);

      if (r > t1)
        return FALSE;

      if (r > t0)
        t0 = r;

      r = div16(edge[i].q2 << BITS, -p);

      if (r < t0)
        return FALSE;

      if (r < t1)
        t1 = r;
    } else {
      r = div16(edge[i].q1 << BITS, p);

      if (r < t0)
        return FALSE;

      if (r < t1)
        t1 = r;

      r = div16(edge[i].q2 << BITS, -p);

      if (r > t1)
        return FALSE;

      if (r > t0)
        t0 = r;
    }
  }

  if (t0 > 0) {
    line->x1 += (t0 * xdelta + HALF) >> BITS;
    line->y1 += (t0 * ydelta + HALF) >> BITS;
  }

  if (t1 < ONE) {
    t1 = ONE - t1;
    line->x2 -= (t1 * xdelta + HALF) >> BITS;
    line->y2 -= (t1 * ydelta + HALF) >> BITS;
  }

  return TRUE;
}
