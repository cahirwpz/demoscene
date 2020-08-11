#include "2d.h"
#include "fx.h"

Box2D ClipWin = { 0, 0, fx4i(319), fx4i(255) };

void PointsInsideBox(Point2D *in, u_char *flags, short n) {
  short *src = (short *)in;

  short minX = ClipWin.minX;
  short minY = ClipWin.minY;
  short maxX = ClipWin.maxX;
  short maxY = ClipWin.maxY;

  while (--n >= 0) {
    short x = *src++;
    short y = *src++;
    u_char f = 0;

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

static bool CheckInside(Point2D *p, u_short plane) {
  if (plane & PF_LEFT)
    return (p->x >= ClipWin.minX);
  if (plane & PF_RIGHT)
    return (p->x < ClipWin.maxX);
  if (plane & PF_TOP)
    return (p->y >= ClipWin.minY);
  if (plane & PF_BOTTOM)
    return (p->y < ClipWin.maxY);
  return false;
}

static void ClipEdge(Point2D *o, Point2D *s, Point2D *e, u_short plane) {
  short dx = s->x - e->x;
  short dy = s->y - e->y;

  if (plane & PF_LEFT) {
    short n = ClipWin.minX - e->x;
    o->x = ClipWin.minX;
    o->y = e->y + div16(dy * n, dx);
  } else if (plane & PF_RIGHT) {
    short n = ClipWin.maxX - e->x;
    o->x = ClipWin.maxX;
    o->y = e->y + div16(dy * n, dx);
  } else if (plane & PF_TOP) {
    short n = ClipWin.minY - e->y;
    o->x = e->x + div16(dx * n, dy);
    o->y = ClipWin.minY;
  } else if (plane & PF_BOTTOM) {
    short n = ClipWin.maxY - e->y;
    o->x = e->x + div16(dx * n, dy);
    o->y = ClipWin.maxY;
  }
}

static u_short ClipPolygon(Point2D *S, Point2D *O, u_short n, u_short plane) {
  Point2D *E = S + 1;

  bool S_inside = CheckInside(S, plane);
  bool needClose = true;
  u_short m = 0;

  if (S_inside) {
    needClose = false;
    O[m++] = *S;
  }

  while (--n) {
    bool E_inside = CheckInside(E, plane);

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

u_short ClipPolygon2D(Point2D *in, Point2D **outp, u_short n, u_short clipFlags)
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
