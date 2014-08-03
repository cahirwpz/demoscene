#ifndef __2D_H__
#define __2D_H__

#include "common.h"
#include "gfx.h"

#define PF_LEFT   1
#define PF_RIGHT  2
#define PF_TOP    4
#define PF_BOTTOM 8

typedef struct {
  WORD x, y;
} Point2D;

typedef struct Box {
  WORD minX, minY;
  WORD maxX, maxY;
} Box2D;

typedef struct {
  WORD m00, m01, x;
  WORD m10, m11, y;
} Matrix2D;

typedef struct {
  WORD sin;
  WORD cos;
} SinCosT;

extern SinCosT sincos[];

__regargs void LoadIdentity2D(Matrix2D *M);
__regargs void Translate2D(Matrix2D *M, WORD x, WORD y);
__regargs void Scale2D(Matrix2D *M, WORD sx, WORD sy);
__regargs void Rotate2D(Matrix2D *M, WORD a);
__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, UWORD n);
__regargs void PointsInsideBox(Point2D *in, UBYTE *flags, UWORD n, Box2D *box);
__regargs BOOL ClipLine2D(Line2D *line, Box2D *box);
__regargs UWORD ClipPolygon2D(Point2D *S, Point2D *O, UWORD n, WORD limit, UWORD plane);

#endif
