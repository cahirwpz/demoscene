#ifndef __2D_H__
#define __2D_H__

#include "common.h"

#define PF_LEFT   8 /* less than zero */
#define PF_RIGHT  4 /* greater or equal to width */
#define PF_TOP    2 /* less than zero */
#define PF_BOTTOM 1 /* greater or equal to height */

typedef struct Point {
  WORD x, y;
} PointT;

typedef struct {
  WORD x1, y1;
  WORD x2, y2;
} LineT;

typedef struct Box {
  WORD minX, minY;
  WORD maxX, maxY;
} BoxT;

typedef struct Edge {
  UWORD p1, p2;
} EdgeT;

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
__regargs void PointsInsideBox(PointT *in, UBYTE *flags, UWORD n, BoxT *box);
__regargs BOOL ClipLine(LineT *line, BoxT *box);

#endif
