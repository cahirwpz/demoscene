#include "2d.h"

__regargs void LoadIdentity2D(Matrix2D *M) {
  M->m00 = 1 << 8;
  M->m01 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = 1 << 8;
  M->y = 0;
}

__regargs void Translate2D(Matrix2D *M, WORD x, WORD y) {
  M->x += x;
  M->y += y;
}

__regargs void Scale2D(Matrix2D *M, WORD sx, WORD sy) {
  M->m00 = (M->m00 * sx) >> 8;
  M->m01 = (M->m01 * sy) >> 8;
  M->m10 = (M->m10 * sx) >> 8;
  M->m11 = (M->m11 * sy) >> 8;
}

__regargs void Rotate2D(Matrix2D *M, WORD a) {
  WORD sin = sincos[a & 0x1ff].sin;
  WORD cos = sincos[a & 0x1ff].cos;
  WORD m00 = M->m00;
  WORD m01 = M->m01;
  WORD m10 = M->m10;
  WORD m11 = M->m11;

  M->m00 = (m00 * cos - m01 * sin) >> 8;
  M->m01 = (m00 * sin + m01 * cos) >> 8;
  M->m10 = (m10 * cos - m11 * sin) >> 8;
  M->m11 = (m10 * sin + m11 * cos) >> 8;
}

#define MULPOINT() {              \
  LONG t0 = (*v++) * x;           \
  LONG t1 = (*v++) * y;           \
  WORD t2 = (*v++);               \
  *dst++ = ((t0 + t1) >> 8) + t2; \
}

__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, UWORD n) {
  WORD *dst = (WORD *)out;
  WORD *src = (WORD *)in;

  while (n--) {
    WORD *v = (WORD *)M;
    WORD x = *src++;
    WORD y = *src++;

    MULPOINT();
    MULPOINT();
  }
}

__regargs void PointsInsideBox(Point2D *in, UBYTE *flags, UWORD n, Box2D *box) {
  WORD *src = (WORD *)in;

  WORD minX = box->minX;
  WORD minY = box->minY;
  WORD maxX = box->maxX;
  WORD maxY = box->maxY;

  while (n--) {
    WORD x = *src++;
    WORD y = *src++;
    UBYTE f = 0;

    if (x < minX)
      f |= PF_LEFT;
    if (x >= maxX)
      f |= PF_RIGHT;
    if (y < minY)
      f |= PF_TOP;
    if (y >= maxY)
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
__regargs BOOL ClipLine2D(Line2D *line, Box2D *box) {
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

static __regargs BOOL CheckInside(Point2D *p, WORD limit, UWORD plane) {
  if (plane & PF_LEFT)
    return (p->x >= limit);
  if (plane & PF_RIGHT)
    return (p->x < limit);
  if (plane & PF_TOP)
    return (p->y >= limit);
  if (plane & PF_BOTTOM)
    return (p->y < limit);
  return FALSE;
}

static __regargs void ClipEdge(Point2D *o, Point2D *s, Point2D *e,
                               WORD limit, UWORD plane) 
{
  WORD dx = s->x - e->x;
  WORD dy = s->y - e->y;

  if (plane & (PF_LEFT | PF_RIGHT)) {
    o->x = limit;
    o->y = e->y + div16(dy * (limit - e->x), dx);
  } 

  if (plane & (PF_TOP | PF_BOTTOM)) {
    o->x = e->x + div16(dx * (limit - e->y), dy);
    o->y = limit;
  }
}

__regargs UWORD ClipPolygon2D(Point2D *S, Point2D *O,
                              UWORD n, WORD limit, UWORD plane)
{
  Point2D *E = S + 1;

  BOOL S_inside = CheckInside(S, limit, plane);
  BOOL needClose = TRUE;
  UWORD m = 0;

  if (S_inside) {
    needClose = FALSE;
    O[m++] = *S;
  }

  while (--n) {
    BOOL E_inside = CheckInside(E, limit, plane);

    if (S_inside && E_inside) {
      O[m++] = *E;
    } else if (S_inside && !E_inside) {
      ClipEdge(&O[m++], S, E, limit, plane);
    } else if (!S_inside && E_inside) {
      ClipEdge(&O[m++], E, S, limit, plane);
      O[m++] = *E;
    }

    S_inside = E_inside;
    S++; E++;
  }

  if (needClose)
    O[m++] = *O;

  return m;
}
