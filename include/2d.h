#ifndef __2D_H__
#define __2D_H__

#include "common.h"
#include "gfx.h"

#define PF_LEFT   1
#define PF_RIGHT  2
#define PF_TOP    4
#define PF_BOTTOM 8

typedef struct IndexList {
  short count;
  short indices[0];
} IndexListT;

typedef struct {
  short m00, m01, x;
  short m10, m11, y;
} Matrix2D;

void LoadIdentity2D(Matrix2D *M);
void Translate2D(Matrix2D *M, short x, short y);
void Scale2D(Matrix2D *M, short sx, short sy);
void Rotate2D(Matrix2D *M, short a);
void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, short n);

extern Box2D ClipWin;

void PointsInsideBox(Point2D *in, u_char *flags, short n);
bool ClipLine2D(Line2D *line);
u_short ClipPolygon2D(Point2D *in, Point2D **outp, u_short n,
                      u_short clipFlags);

typedef struct Shape {
  short points;
  short polygons;

  Point2D *origPoint;
  Point2D *viewPoint;
  u_char *viewPointFlags;
  IndexListT **polygon;
  u_char *polygonFlags;
} ShapeT;

#endif
