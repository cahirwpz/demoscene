#include "memory.h"
#include "file.h"
#include "reader.h"
#include "3d.h"
#include "fx.h"

Frustum3D ClipFrustum;

__regargs void LoadIdentity3D(Matrix3D *M) {
  M->m00 = fx12f(1.0);
  M->m01 = 0;
  M->m02 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = fx12f(1.0);
  M->m12 = 0;
  M->y = 0;

  M->m10 = 0;
  M->m11 = 0;
  M->m12 = fx12f(1.0);
  M->y = 0;
}

__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z) {
  M->x += x;
  M->y += y;
  M->z += z;
}

__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m02 = normfx(M->m02 * sz);

  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
  M->m12 = normfx(M->m12 * sz);

  M->m20 = normfx(M->m20 * sx);
  M->m21 = normfx(M->m21 * sy);
  M->m22 = normfx(M->m22 * sz);
}

#define MULROW() {            \
  WORD *c = a;                \
  LONG t0 = (*c++) * (*b++);  \
  LONG t1 = (*c++) * (*b++);  \
  LONG t2 = (*c++) * (*b++);  \
  *m++ = normfx(t0 + t1 + t2); \
}

/* Assumes "b" is a transposed 3x3 matrix! */
static __regargs void Compose3D(Matrix3D *d, WORD a[9], WORD b[9]) {
  WORD *m = &d->m00;

  pushl(b);
  MULROW();
  MULROW();
  MULROW();
  popl(b);
  a += 3;
  m++;

  pushl(b);
  MULROW();
  MULROW();
  MULROW();
  popl(b);
  a += 3;
  m++;

  MULROW();
  MULROW();
  MULROW();
}

__regargs void LoadRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  WORD sinX = SIN(ax);
  WORD cosX = COS(ax);
  WORD sinY = SIN(ay);
  WORD cosY = COS(ay);
  WORD sinZ = SIN(az);
  WORD cosZ = COS(az);

  WORD tmp0 = normfx(sinX * sinY);
  WORD tmp1 = normfx(cosX * sinY);

  M->m00 = normfx(cosY * cosZ);
  M->m01 = normfx(cosY * sinZ);
  M->m02 = -sinY;
  M->x = 0;

  M->m10 = normfx(tmp0 * cosZ - cosX * sinZ);
  M->m11 = normfx(tmp0 * sinZ + cosX * cosZ);
  M->m12 = normfx(sinX * cosY);
  M->y = 0;

  M->m20 = normfx(tmp1 * cosZ + sinX * sinZ);
  M->m21 = normfx(tmp1 * sinZ - sinX * cosZ);
  M->m22 = normfx(cosX * cosY);
  M->z = 0;
}

__regargs void Rotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  WORD ma[9], mb[9];

  ma[0] = M->m00;
  ma[1] = M->m01;
  ma[2] = M->m02;
  ma[3] = M->m10;
  ma[4] = M->m11;
  ma[5] = M->m12;
  ma[6] = M->m20;
  ma[7] = M->m21;
  ma[8] = M->m22;

  {
    WORD sinX = SIN(ax);
    WORD cosX = COS(ax);
    WORD sinY = SIN(ay);
    WORD cosY = COS(ay);
    WORD sinZ = SIN(az);
    WORD cosZ = COS(az);

    WORD tmp0 = normfx(sinX * sinY);
    WORD tmp1 = normfx(cosX * sinY);

    mb[0] = normfx(cosY * cosZ);
    mb[3] = normfx(cosY * sinZ);
    mb[6] = -sinY;
    mb[1] = normfx(tmp0 * cosZ - cosX * sinZ);
    mb[4] = normfx(tmp0 * sinZ + cosX * cosZ);
    mb[7] = normfx(sinX * cosY);
    mb[2] = normfx(tmp1 * cosZ + sinX * sinZ);
    mb[5] = normfx(tmp1 * sinZ - sinX * cosZ);
    mb[8] = normfx(cosX * cosY);
  }

  Compose3D(M, ma, mb);
}

#define MULVERTEX() {                 \
  LONG t0 = (*v++) * x;               \
  LONG t1 = (*v++) * y;               \
  LONG t2 = (*v++) * z;               \
  LONG t3 = (*v++);                   \
  *dst++ = normfx(t0 + t1 + t2) + t3; \
}

__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, UWORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;
  WORD *v = (WORD *)M;

  while (n--) {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;

    pushl(v);
    MULVERTEX();
    MULVERTEX();
    MULVERTEX();
    popl(v);
  }
}

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


__regargs Object3D *NewObject3D(UWORD points, UWORD polygons) {
  Object3D *object = AllocMemSafe(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);

  object->points = points;
  object->polygons = polygons;

  object->point = AllocMemSafe(sizeof(Point3D) * points, MEMF_PUBLIC);
  object->cameraPoint = AllocMemSafe(sizeof(Point3D) * points, MEMF_PUBLIC);
  object->cameraPointFlags = AllocMemSafe(points, MEMF_PUBLIC);
  object->polygon = AllocMemSafe(sizeof(PolygonT) * polygons, MEMF_PUBLIC);
  object->polygonNormal =
    AllocMemSafe(sizeof(Point3D) * polygons, MEMF_PUBLIC);

  return object;
}

__regargs void DeleteObject3D(Object3D *object) {
  if (object->polygonVertex)
    FreeMem(object->polygonVertex, sizeof(UWORD) * object->polygonVertices);
  FreeMem(object->polygonNormal, sizeof(Point3D) * object->polygons);
  FreeMem(object->polygon, sizeof(PolygonT) * object->polygons);
  FreeMem(object->cameraPointFlags, object->points);
  FreeMem(object->cameraPoint, sizeof(Point3D) * object->points);
  FreeMem(object->point, sizeof(Point3D) * object->points);
  FreeMem(object, sizeof(Object3D));
}

__regargs Object3D *LoadObject3D(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  Object3D *object = NULL;
  WORD i, j, points, edges;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &points) && ReadNumber(&data, &edges)) {
    object = NewObject3D(points, edges);

    for (i = 0; i < object->points; i++) {
      if (!ReadNumber(&data, &object->point[i].x) ||
          !ReadNumber(&data, &object->point[i].y) ||
          !ReadNumber(&data, &object->point[i].z))
        goto error;

      object->point[i].x *= 16;
      object->point[i].y *= 16;
      object->point[i].z *= 16;
    }

    /* Calculate size of polygonVertex array. */
    {
      char *ptr = data;

      for (i = 0; i < object->polygons; i++) {
        WORD n, tmp;

        if (!ReadNumber(&ptr, &n))
          goto error;

        object->polygonVertices += n + 1;

        while (n--) {
          if (!ReadNumber(&ptr, &tmp))
            goto error;
        }
      }
    }

    object->polygonVertex =
      AllocMemSafe(sizeof(UWORD) * object->polygonVertices, MEMF_PUBLIC);

    for (i = 0, j = 0; i < object->polygons; i++) {
      UWORD n, k;

      if (!ReadNumber(&data, &n))
        goto error;

      Log("Polygon %ld at %ld:", (LONG)i, (LONG)j);

      object->polygon[i].vertices = n + 1;
      object->polygon[i].index = j;

      k = j;

      while (n--) {
        UWORD tmp;

        if (!ReadNumber(&data, &tmp))
          goto error;

        object->polygonVertex[j++] = tmp;
        Log(" %ld", (LONG)tmp);
      }
      object->polygonVertex[j++] = object->polygonVertex[k];

      Log(" %ld\n", (LONG)object->polygonVertex[k]);
    }

    FreeAutoMem(file);
    return object;
  }

error:
  DeleteObject3D(object);
  FreeAutoMem(file);
  return NULL;
}

/*
 * For given triangle T with vertices A, B and C, surface normal N is a cross
 * product between vectors AB and BC.
 *
 * Ordering of vertices in polygon description is meaningful - depending on
 * that the normal vector will be directed inwards or outwards.
 *
 * Clockwise convention is used.
 */

__regargs void UpdatePolygonNormals(Object3D *object) {
  WORD polygons = object->polygons;
  Point3D *point = object->cameraPoint;
  PolygonT *polygon = object->polygon;
  WORD *normal = (WORD *)object->polygonNormal;
  UWORD *vertex = object->polygonVertex;

  while (polygons--) {
    UWORD *v = &vertex[polygon->index];

    Point3D *p1 = &point[*v++];
    Point3D *p2 = &point[*v++];
    Point3D *p3 = &point[*v++];

    WORD ax = p1->x - p2->x;
    WORD ay = p1->y - p2->y;
    WORD az = p1->z - p2->z;
    WORD bx = p2->x - p3->x;
    WORD by = p2->y - p3->y;
    WORD bz = p2->z - p3->z;

    *normal++ = normfx(ay * bz - by * az);
    *normal++ = normfx(az * bx - bz * ax);
    *normal++ = normfx(ax * by - bx * ay);

    polygon++;
  }
}
