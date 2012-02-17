#ifndef __GFX_VECTOR3D_H__
#define __GFX_VECTOR3D_H__

#include "std/types.h"

typedef struct {
  float x, y, z;
} Vector3D;

void V3D_Add(Vector3D *d, Vector3D *a, Vector3D *b);
void V3D_Sub(Vector3D *d, Vector3D *a, Vector3D *b);
void V3D_Cross(Vector3D *d, Vector3D *a, Vector3D *b);
float V3D_Dot(Vector3D *a, Vector3D *b);
float V3D_Length(Vector3D *a);
void V3D_Scale(Vector3D *d, Vector3D *a, float s);
void V3D_Normalize(Vector3D *d, Vector3D *a, float l);

#endif
