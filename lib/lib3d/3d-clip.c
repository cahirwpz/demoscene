#include "3d.h"

Frustum3D ClipFrustum;

void PointsInsideFrustum(Point3D *in, u_char *flags, u_short n) {
  short *src = (short *)in;

  while (n--) {
    short x = *src++;
    short y = *src++;
    short z = *src++;
    u_char f = 0;

    if (x < z)
      f |= PF_LEFT;
    if (x > -z)
      f |= PF_RIGHT;
    if (y < z)
      f |= PF_TOP;
    if (y > -z)
      f |= PF_BOTTOM;
    if (z > ClipFrustum.near)
      f |= PF_NEAR;
    if (z < ClipFrustum.far)
      f |= PF_FAR;

    *flags++ = f;
  }
}

static bool CheckInside(Point3D *p, u_short plane) {
  if (plane & PF_LEFT)
    return (p->x > p->z);
  if (plane & PF_RIGHT)
    return (p->x < -p->z);
  if (plane & PF_TOP)
    return (p->y > p->z);
  if (plane & PF_BOTTOM)
    return (p->y < -p->z);
  if (plane & PF_NEAR)
    return (p->z < ClipFrustum.near);
  if (plane & PF_FAR)
    return (p->z > ClipFrustum.far);
  return false;
}

static void ClipEdge(Point3D *o, Point3D *s, Point3D *e, u_short plane) {
  short dx = e->x - s->x;
  short dy = e->y - s->y;
  short dz = e->z - s->z;

#if 0
  if (plane & PF_LEFT) {
    short n = s->z - s->x;
    short d = dx - dz;

    o->x = e->x + div16(dx * n, d);
    o->y = e->y + div16(dy * n, d);
    o->z = e->z + div16(dz * n, d);
  } else if (plane & PF_RIGHT) {
  } else if (plane & PF_TOP) {
  } else if (plane & PF_BOTTOM) {
  }
#endif
  if (plane & PF_NEAR) {
    short n = ClipFrustum.near - s->z;

    o->x = s->x + div16(dx * n, dz);
    o->y = s->y + div16(dy * n, dz);
    o->z = ClipFrustum.near;
  } else if (plane & PF_FAR) {
    short n = ClipFrustum.far - s->z;

    o->x = s->x + div16(dx * n, dz);
    o->y = s->y + div16(dy * n, dz);
    o->z = ClipFrustum.far;
  }
}

static u_short ClipPolygon(Point3D *S, Point3D *O, u_short n, u_short plane) {
  Point3D *E = S + 1;

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

u_short ClipPolygon3D(Point3D *in, Point3D **outp, u_short n, u_short clipFlags)
{
  Point3D *out = *outp;

  if (clipFlags) {
#if 0
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
#endif
    if (clipFlags & PF_NEAR) {
      n = ClipPolygon(in, out, n, PF_NEAR);
      swapr(in, out);
    }
    if (clipFlags & PF_FAR) {
      n = ClipPolygon(in, out, n, PF_FAR);
      swapr(in, out);
    }
  }

  *outp = in;
  return n;
}
