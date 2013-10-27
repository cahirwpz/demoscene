#ifndef __3D_H__
#define __3D_H__

#include <exec/types.h>

#define PF_LEFT   8 /* less than zero */
#define PF_RIGHT  4 /* greater or equal to width */
#define PF_TOP    2 /* less than zero */
#define PF_BOTTOM 1 /* greater or equal to height */

typedef struct {
  UWORD p1, p2;
} EdgeT;

typedef struct {
  WORD x, y, z;
} VertexT;

typedef struct {
  WORD x, y;
} PointT;

typedef struct {
  WORD x1, y1;
  WORD x2, y2;
} LineT;

typedef struct {
  WORD m[9];
  WORD viewerX;
  WORD viewerY;
  WORD viewerZ;
  WORD rotateX;
  WORD rotateY;
  WORD rotateZ;
} View3D;

typedef struct {
  WORD sin;
  WORD cos;
} SinCosT;

typedef struct {
  UWORD nVertex;
  UWORD nEdge;

  VertexT *vertex;
  EdgeT *edge;

  PointT *point;
  UBYTE *pointFlags;

  LineT *line;
  UBYTE *lineFlags;
} Object3D;

extern const SinCosT *const sincos;

void CalculateView3D(View3D *view asm("a0"));
void TransformVertices(View3D *view asm("a0"), Object3D *object asm("a1"));
void ClipEdges(Object3D *object asm("a0"));

#endif
