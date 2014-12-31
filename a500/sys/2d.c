#include "2d.h"
#include "memory.h"
#include "file.h"
#include "reader.h"
#include "fx.h"

Box2D ClipWin = { 0, 0, fx4i(319), fx4i(255) };

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

__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, WORD n) {
  WORD *dst = (WORD *)out;
  WORD *src = (WORD *)in;
  WORD *v = (WORD *)M;
  WORD m00 = *v++;
  WORD m01 = *v++;
  WORD mx  = *v++;
  WORD m10 = *v++;
  WORD m11 = *v++;
  WORD my  = *v++;

  while (--n >= 0) {
    WORD x = *src++;
    WORD y = *src++;

    *dst++ = normfx(m00 * x + m01 * y) + mx;
    *dst++ = normfx(m10 * x + m11 * y) + my;
  }
}

__regargs BOOL ClipArea2D(Point2D *dst, WORD width, WORD height, Area2D *src) {
  if (dst->y < 0) {
    src->y -= dst->y;
    src->h += dst->y;
    dst->y = 0;
  }

  if (dst->x < 0) {
    src->x -= dst->x;
    src->w += dst->x;
    dst->x = 0;
  }

  if (dst->y + src->h >= height)
    src->h = height - dst->y;

  if (dst->x + src->w >= width)
    src->w = width - dst->x;

  return (src->w > 0 && src->h > 0);
}

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

__regargs ShapeT *NewShape(UWORD points, UWORD polygons) {
  ShapeT *shape = MemAlloc(sizeof(ShapeT), MEMF_PUBLIC|MEMF_CLEAR);

  shape->points = points;
  shape->polygons = polygons;

  shape->origPoint = MemAlloc(sizeof(Point2D) * points, MEMF_PUBLIC);
  shape->viewPoint = MemAlloc(sizeof(Point2D) * points, MEMF_PUBLIC);
  shape->viewPointFlags = MemAlloc(points, MEMF_PUBLIC);
  shape->polygon = MemAlloc(sizeof(PolygonT) * polygons, MEMF_PUBLIC);

  return shape;
}

__regargs void DeleteShape(ShapeT *shape) {
  MemFree(shape->polygonVertex, sizeof(UWORD) * shape->polygonVertices);
  MemFree(shape->polygon, sizeof(PolygonT) * shape->polygons);
  MemFree(shape->viewPointFlags, shape->points);
  MemFree(shape->viewPoint, sizeof(Point2D) * shape->points);
  MemFree(shape->origPoint, sizeof(Point2D) * shape->points);
  MemFree(shape, sizeof(ShapeT));
}

__regargs ShapeT *LoadShape(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  WORD points = 0, polygons = 0;
  ShapeT *shape = NULL;

  if (file) {
    char *data = file;
    WORD n;

    if (ReadNumber(&data, NULL) && ReadNumber(&data, NULL)) {
      while (*data) {
        if (!ReadNumber(&data, &n))
          break;

        points += n;
        polygons++;

        n *= 2;

        do {
          if (!ReadNumber(&data, NULL))
            break;
        } while (--n > 0);

        if (n > 0)
          break;

        data = SkipSpaces(data);
      }
    }

    if (*data == 0) {
      shape = NewShape(points, polygons);

      shape->polygonVertices = points + polygons;
      shape->polygonVertex = MemAlloc(sizeof(UWORD) * shape->polygonVertices,
                                      MEMF_PUBLIC);
    }
  }

  if (shape) {
    char *data = file;
    WORD i = 0, j = 0, k = 0;
    WORD origin_x, origin_y;

    ReadNumber(&data, &origin_x);
    ReadNumber(&data, &origin_y);

    while (*data) {
      WORD n, old_k;

      ReadNumber(&data, &n);

      shape->polygon[i].vertices = n + 1;
      shape->polygon[i].index = j;

      old_k = k;

      while (--n >= 0) {
        WORD x, y;

        ReadNumber(&data, &x);
        ReadNumber(&data, &y);

        shape->origPoint[k].x = (x - origin_x) * 16;
        shape->origPoint[k].y = (y - origin_y) * 16;
        shape->polygonVertex[j] = k;

        j++; k++;
      }

      shape->polygonVertex[j++] = old_k;

      i++;

      data = SkipSpaces(data);
    }
  }

  MemFreeAuto(file);
  return shape;
}

