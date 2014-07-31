#ifndef __3D_H__
#define __3D_H__

#include "2d.h"

typedef struct {
  WORD x, y, z;
} VertexT;

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
  UWORD nVertex;
  UWORD nEdge;

  VertexT *vertex;
  EdgeT *edge;

  PointT *point;
  UBYTE *pointFlags;

  LineT *line;
  UBYTE *lineFlags;
} Object3D;

void CalculateView3D(View3D *view asm("a0"));
void TransformVertices(View3D *view asm("a0"), Object3D *object asm("a1"));

#endif
