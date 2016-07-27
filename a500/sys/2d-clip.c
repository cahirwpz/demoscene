#include "2d.h"
#include "fx.h"

Box2D ClipWin = { 0, 0, fx4i(319), fx4i(255) };

__regargs void PointsInsideBox(Point2D *in, UBYTE *flags, WORD n) {
  WORD *src = (WORD *)in;

  WORD minX = ClipWin.minX;
  WORD minY = ClipWin.minY;
  WORD maxX = ClipWin.maxX;
  WORD maxY = ClipWin.maxY;

  while (--n >= 0) {
    WORD x = *src++;
    WORD y = *src++;
    UBYTE f = 0;

    if (x < minX)
      f |= PF_LEFT;
    else if (x >= maxX)
      f |= PF_RIGHT;
    if (y < minY)
      f |= PF_TOP;
    else if (y >= maxY)
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

/* Liang-Barsky algorithm. */
__regargs BOOL ClipLine2D(Line2D *line) {
  WORD t0 = 0;
  WORD t1 = ONE;
  WORD xdelta = line->x2 - line->x1;
  WORD ydelta = line->y2 - line->y1;
  WORD i;

  edge[0].p  = -xdelta;
  edge[0].q1 = line->x1 - ClipWin.minX;
  edge[0].q2 = ClipWin.maxX - line->x1;
  edge[1].p  = -ydelta;
  edge[1].q1 = line->y1 - ClipWin.minY;
  edge[1].q2 = ClipWin.maxY - line->y1;

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

static __regargs BOOL CheckInside(Point2D *p, UWORD plane) {
  if (plane & PF_LEFT)
    return (p->x >= ClipWin.minX);
  if (plane & PF_RIGHT)
    return (p->x < ClipWin.maxX);
  if (plane & PF_TOP)
    return (p->y >= ClipWin.minY);
  if (plane & PF_BOTTOM)
    return (p->y < ClipWin.maxY);
  return FALSE;
}

static __regargs void ClipEdge(Point2D *o, Point2D *s, Point2D *e, UWORD plane)
{
  WORD dx = s->x - e->x;
  WORD dy = s->y - e->y;

  if (plane & PF_LEFT) {
    WORD n = ClipWin.minX - e->x;
    o->x = ClipWin.minX;
    o->y = e->y + div16(dy * n, dx);
  } else if (plane & PF_RIGHT) {
    WORD n = ClipWin.maxX - e->x;
    o->x = ClipWin.maxX;
    o->y = e->y + div16(dy * n, dx);
  } else if (plane & PF_TOP) {
    WORD n = ClipWin.minY - e->y;
    o->x = e->x + div16(dx * n, dy);
    o->y = ClipWin.minY;
  } else if (plane & PF_BOTTOM) {
    WORD n = ClipWin.maxY - e->y;
    o->x = e->x + div16(dx * n, dy);
    o->y = ClipWin.maxY;
  }
}

static __regargs UWORD ClipPolygon(Point2D *S, Point2D *O,
                                   UWORD n, UWORD plane)
{
  Point2D *E = S + 1;

  BOOL S_inside = CheckInside(S, plane);
  BOOL needClose = TRUE;
  UWORD m = 0;

  if (S_inside) {
    needClose = FALSE;
    O[m++] = *S;
  }

  while (--n) {
    BOOL E_inside = CheckInside(E, plane);

    if (S_inside && E_inside) {
      O[m++] = *E;
    } else if (S_inside && !E_inside) {
      ClipEdge(&O[m++], S, E, plane);
    } else if (!S_inside && E_inside) {
      ClipEdge(&O[m++], E, S, plane);
      O[m++] = *E;
    }

    S_inside = E_inside;
    S++; E++;
  }

  if (needClose)
    O[m++] = *O;

  return m;
}

__regargs UWORD ClipPolygon2D(Point2D *in, Point2D **outp, UWORD n,
                              UWORD clipFlags)
{
  Point2D *out = *outp;

  if (clipFlags) {
    if (clipFlags & PF_LEFT) {
      n = ClipPolygon(in, out, n, PF_LEFT);
      swapr(in, out);
    }
    if (clipFlags & PF_TOP) {
      n = ClipPolygon(in, out, n, PF_TOP);
      swapr(in, out);
    }
    if (clipFlags & PF_RIGHT) {
      n = ClipPolygon(in, out, n, PF_RIGHT);
      swapr(in, out);
    }
    if (clipFlags & PF_BOTTOM) {
      n = ClipPolygon(in, out, n, PF_BOTTOM);
      swapr(in, out);
    }
  }

  *outp = in;
  return n;
}
