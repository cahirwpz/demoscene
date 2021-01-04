#include <2d.h>

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
