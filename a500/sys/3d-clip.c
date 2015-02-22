#include "3d.h"

Frustum3D ClipFrustum;

__regargs void PointsInsideFrustum(Point3D *in, UBYTE *flags, UWORD n) {
  WORD *src = (WORD *)in;

  while (n--) {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;
    UBYTE f = 0;

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

static __regargs BOOL CheckInside(Point3D *p, UWORD plane) {
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
  return FALSE;
}

static __regargs void ClipEdge(Point3D *o, Point3D *s, Point3D *e, UWORD plane)
{
  WORD dx = e->x - s->x;
  WORD dy = e->y - s->y;
  WORD dz = e->z - s->z;

#if 0
  if (plane & PF_LEFT) {
    WORD n = s->z - s->x;
    WORD d = dx - dz;

    o->x = e->x + div16(dx * n, d);
    o->y = e->y + div16(dy * n, d);
    o->z = e->z + div16(dz * n, d);
  } else if (plane & PF_RIGHT) {
  } else if (plane & PF_TOP) {
  } else if (plane & PF_BOTTOM) {
  }
#endif
  if (plane & PF_NEAR) {
    WORD n = ClipFrustum.near - s->z;

    o->x = s->x + div16(dx * n, dz);
    o->y = s->y + div16(dy * n, dz);
    o->z = ClipFrustum.near;
  } else if (plane & PF_FAR) {
    WORD n = ClipFrustum.far - s->z;

    o->x = s->x + div16(dx * n, dz);
    o->y = s->y + div16(dy * n, dz);
    o->z = ClipFrustum.far;
  }
}

static __regargs UWORD ClipPolygon(Point3D *S, Point3D *O,
                                   UWORD n, UWORD plane)
{
  Point3D *E = S + 1;

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
__regargs UWORD ClipPolygon3D(Point3D *in, Point3D **outp, UWORD n,
                              UWORD clipFlags)
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
