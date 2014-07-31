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
  WORD m00, m01, x;
  WORD m10, m11, y;
} View2D;

typedef struct {
  WORD sin;
  WORD cos;
} SinCosT;

extern SinCosT sincos[];

__regargs void Identity2D(View2D *view);
__regargs void Translate2D(View2D *view, WORD x, WORD y);
__regargs void Scale2D(View2D *view, WORD sx, WORD sy);
__regargs void Rotate2D(View2D *view, WORD a);
__regargs void Transform2D(View2D *view, PointT *out, PointT *in, UWORD n);

#endif
