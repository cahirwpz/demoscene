#include <2d.h>
#include <fx.h>

#define BITS 8
#define ONE (1 << BITS)
#define HALF (1 << (BITS - 1))

typedef struct {
  short p, q1, q2;
} LBEdgeT;

static LBEdgeT edge[2];

/* Liang-Barsky algorithm. */
bool ClipLine2D(Line2D *line) {
  short t0 = 0;
  short t1 = ONE;
  short xdelta = line->x2 - line->x1;
  short ydelta = line->y2 - line->y1;
  short i;

  edge[0].p  = -xdelta;
  edge[0].q1 = line->x1 - ClipWin.minX;
  edge[0].q2 = ClipWin.maxX - line->x1;
  edge[1].p  = -ydelta;
  edge[1].q1 = line->y1 - ClipWin.minY;
  edge[1].q2 = ClipWin.maxY - line->y1;

  for (i = 0; i < 2; i++) {
    short p = edge[i].p;
    short r;

    if (p == 0)
      continue;

    if (p < 0) {
      r = div16(edge[i].q1 << BITS, p);

      if (r > t1)
        return false;

      if (r > t0)
        t0 = r;

      r = div16(edge[i].q2 << BITS, -p);

      if (r < t0)
        return false;

      if (r < t1)
        t1 = r;
    } else {
      r = div16(edge[i].q1 << BITS, p);

      if (r < t0)
        return false;

      if (r < t1)
        t1 = r;

      r = div16(edge[i].q2 << BITS, -p);

      if (r > t1)
        return false;

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

  return true;
}
