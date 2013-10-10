#ifndef __3D_H__
#define __3D_H__

#include <exec/types.h>

typedef struct {
  WORD x, y, z;
} VertexT;

typedef struct {
  WORD x, y;
} PointT;

typedef struct {
  WORD m[9];
  WORD viewerX;
  WORD viewerY;
  WORD viewerZ;
} View3D;

typedef struct {
  WORD sin;
  WORD cos;
} SinCosT;

extern const SinCosT *const sincos;

void CalculateView3D(View3D *view asm("a0"), WORD angle_x asm("d0"),
                     WORD angle_y asm("d1"), WORD angle_z asm("d2"));

void TransformVertices(View3D *view asm("a0"), VertexT *vertex asm("a1"),
                       PointT *point asm("a2"), WORD n asm("d0"));

#endif
