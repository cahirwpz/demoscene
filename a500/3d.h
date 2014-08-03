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
  UWORD p1, p2;
} EdgeT;

typedef struct {
  UWORD points;
  UWORD edges;

  Point3D *point;
  EdgeT *edge;

  Point3D *cameraPoint;
  UBYTE *frustumPointFlags;
} Object3D;

__regargs void LoadIdentity3D(Matrix3D *M);
__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z);
__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz);
__regargs void LoadRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az);
__regargs void Rotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az);
__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, UWORD n);
__regargs void PointsInsideFrustum(Point3D *in, UBYTE *flags, UWORD n,
                                   WORD near, WORD far);

#endif
