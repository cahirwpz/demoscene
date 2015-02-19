#ifndef __3D_H__
#define __3D_H__

#include "2d.h"

#define PF_NEAR 16
#define PF_FAR  32

typedef struct {
  WORD x, y, z;
} Point3D;

typedef struct {
  WORD m00, m01, m02, x;
  WORD m10, m11, m12, y;
  WORD m20, m21, m22, z;
} Matrix3D;

typedef struct {
  WORD near;
  WORD far;
} Frustum3D;

__regargs void LoadIdentity3D(Matrix3D *M);
__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z);
__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz);
__regargs void LoadRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az);
__regargs void Rotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az);
__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, WORD n);

extern Frustum3D ClipFrustum;

__regargs void PointsInsideFrustum(Point3D *in, UBYTE *flags, UWORD n);
__regargs UWORD ClipPolygon3D(Point3D *in, Point3D **outp, UWORD n,
                              UWORD clipFlags);

typedef struct {
  UWORD points;
  UWORD polygons;
  UWORD edges;

  Point3D *point;
  Point3D *cameraPoint;
  Point2D *screenPoint;
  BYTE *pointFlags;

  IndexListT **polygon;
  Point3D *polygonNormal;
  BYTE *polygonFlags;
  WORD *polygonData;

  IndexListT **vertexPolygon;
  WORD *vertexPolygonData;

  EdgeT *edge;
  IndexListT **edgePolygon;
  WORD *edgePolygonData;
} Object3D;

__regargs Object3D *NewObject3D(UWORD points, UWORD polygons);
__regargs void DeleteObject3D(Object3D *object);
__regargs Object3D *LoadLWO(char *filename, FLOAT scale);

__regargs void UpdatePolygonNormals(Object3D *object);
__regargs void NormalizePolygonNormals(Object3D *object);
__regargs void CalculateEdges(Object3D *object);
__regargs void CalculateVertexPolygonMap(Object3D *object);

#endif
