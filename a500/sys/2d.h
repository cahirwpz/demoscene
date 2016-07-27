#ifndef __2D_H__
#define __2D_H__

#include "common.h"
#include "gfx.h"

#define PF_LEFT   1
#define PF_RIGHT  2
#define PF_TOP    4
#define PF_BOTTOM 8

typedef struct Edge {
  UWORD p0, p1;
} EdgeT;

typedef struct IndexList {
  WORD count;
  WORD indices[0];
} IndexListT;

typedef struct {
  WORD m00, m01, x;
  WORD m10, m11, y;
} Matrix2D;

__regargs void LoadIdentity2D(Matrix2D *M);
__regargs void Translate2D(Matrix2D *M, WORD x, WORD y);
__regargs void Scale2D(Matrix2D *M, WORD sx, WORD sy);
__regargs void Rotate2D(Matrix2D *M, WORD a);
__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, WORD n);

extern Box2D ClipWin;

__regargs void PointsInsideBox(Point2D *in, UBYTE *flags, WORD n);
__regargs BOOL ClipLine2D(Line2D *line);
__regargs UWORD ClipPolygon2D(Point2D *in, Point2D **outp, UWORD n,
                              UWORD clipFlags);

typedef struct Shape {
  WORD points;
  WORD polygons;

  Point2D *origPoint;
  Point2D *viewPoint;
  UBYTE *viewPointFlags;
  IndexListT **polygon;
  UBYTE *polygonFlags;
  WORD *polygonData;
} ShapeT;

__regargs ShapeT *LoadShape(char *filename);
__regargs void DeleteShape(ShapeT *shape);

#endif
