#include "2d.h"
#include "memory.h"
#include "file.h"
#include "reader.h"
#include "fx.h"

Box2D ClipWin;

__regargs void LoadIdentity2D(Matrix2D *M) {
  M->m00 = fx12f(1.0);
  M->m01 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = fx12f(1.0);
  M->y = 0;
}

__regargs void Translate2D(Matrix2D *M, WORD x, WORD y) {
  M->x += x;
  M->y += y;
}

__regargs void Scale2D(Matrix2D *M, WORD sx, WORD sy) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
}

__regargs void Rotate2D(Matrix2D *M, WORD a) {
  WORD sin = SIN(a);
  WORD cos = COS(a);
  WORD m00 = M->m00;
  WORD m01 = M->m01;
  WORD m10 = M->m10;
  WORD m11 = M->m11;

  M->m00 = normfx(m00 * cos - m01 * sin);
  M->m01 = normfx(m00 * sin + m01 * cos);
  M->m10 = normfx(m10 * cos - m11 * sin);
  M->m11 = normfx(m10 * sin + m11 * cos);
}

#define MULPOINT() {             \
  LONG t0 = (*v++) * x;          \
  LONG t1 = (*v++) * y;          \
  WORD t2 = (*v++);              \
  *dst++ = normfx(t0 + t1) + t2; \
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

__regargs void PointsInsideBox(Point2D *in, UBYTE *flags, UWORD n) {
  WORD *src = (WORD *)in;

  WORD minX = ClipWin.minX;
  WORD minY = ClipWin.minY;
  WORD maxX = ClipWin.maxX;
  WORD maxY = ClipWin.maxY;

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
    return (p->y < ClipWin.maxX);
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

__regargs ShapeT *NewShape(UWORD points, UWORD polygons) {
  ShapeT *shape = AllocMemSafe(sizeof(ShapeT), MEMF_PUBLIC|MEMF_CLEAR);

  shape->points = points;
  shape->polygons = polygons;

  shape->origPoint = AllocMemSafe(sizeof(Point2D) * points, MEMF_PUBLIC);
  shape->viewPoint = AllocMemSafe(sizeof(Point2D) * points, MEMF_PUBLIC);
  shape->viewPointFlags = AllocMemSafe(points, MEMF_PUBLIC);
  shape->polygon = AllocMemSafe(sizeof(PolygonT) * polygons, MEMF_PUBLIC);

  return shape;
}

__regargs void DeleteShape(ShapeT *shape) {
  if (shape->polygonVertex)
    FreeMem(shape->polygonVertex, sizeof(UWORD) * shape->polygonVertices);
  FreeMem(shape->polygon, sizeof(PolygonT) * shape->polygons);
  FreeMem(shape->viewPointFlags, shape->points);
  FreeMem(shape->viewPoint, sizeof(Point2D) * shape->points);
  FreeMem(shape->origPoint, sizeof(Point2D) * shape->points);
  FreeMem(shape, sizeof(ShapeT));
}

__regargs ShapeT *LoadShape(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  ShapeT *shape = NULL;
  WORD i, j, points, polygons;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &points) && ReadNumber(&data, &polygons)) {
    shape = NewShape(points, polygons);

    for (i = 0; i < shape->points; i++) {
      if (!ReadNumber(&data, &shape->origPoint[i].x) ||
          !ReadNumber(&data, &shape->origPoint[i].y))
        goto error;

      shape->origPoint[i].x *= 16;
      shape->origPoint[i].y *= 16;
    }

    /* Calculate size of polygonVertex array. */
    {
      char *ptr = data;

      for (i = 0; i < shape->polygons; i++) {
        WORD n, tmp;

        if (!ReadNumber(&ptr, &n))
          goto error;

        shape->polygonVertices += n + 1;

        while (n--) {
          if (!ReadNumber(&ptr, &tmp))
            goto error;
        }
      }
    }

    shape->polygonVertex =
      AllocMemSafe(sizeof(UWORD) * shape->polygonVertices, MEMF_PUBLIC);

    for (i = 0, j = 0; i < shape->polygons; i++) {
      UWORD n, k;

      if (!ReadNumber(&data, &n))
        goto error;

      Log("Polygon %ld at %ld:", (LONG)i, (LONG)j);

      shape->polygon[i].vertices = n + 1;
      shape->polygon[i].index = j;

      k = j;

      while (n--) {
        UWORD tmp;

        if (!ReadNumber(&data, &tmp))
          goto error;

        shape->polygonVertex[j++] = tmp;
        Log(" %ld", (LONG)tmp);
      }
      shape->polygonVertex[j++] = shape->polygonVertex[k];

      Log(" %ld\n", (LONG)shape->polygonVertex[k]);
    }

    FreeAutoMem(file);
    return shape;
  }

error:
  DeleteShape(shape);
  FreeAutoMem(file);
  return NULL;
}

