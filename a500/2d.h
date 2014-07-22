#ifndef __2D_H__
#define __2D_H__

#include <exec/types.h>

typedef struct Point {
  WORD x, y;
} PointT;

typedef struct Edge {
  UWORD p1, p2;
} EdgeT;

typedef struct Shape {
  UWORD nPoints;
  UWORD nEdges;

  PointT *points;
  PointT *outPoints;

  EdgeT *edges;
} ShapeT;

typedef struct {
  WORD m00, m01, m10, m11;
  WORD x, y;
} Transform2D;

typedef struct {
  WORD sin;
  WORD cos;
} SinCosT;

extern SinCosT sincos[];

__regargs void Identity2D(Transform2D *t);
__regargs void Translate2D(Transform2D *t, WORD x, WORD y);
__regargs void Scale2D(Transform2D *t, WORD sx, WORD sy);
__regargs void Rotate2D(Transform2D *t, WORD a);
__regargs void Apply2D(Transform2D *t, PointT *out, PointT *in, UWORD n);

#endif
