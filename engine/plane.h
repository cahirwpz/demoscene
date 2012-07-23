#ifndef __ENGINE_PLANE_H__
#define __ENGINE_PLANE_H__

#include "gfx/vector3d.h"

typedef struct Plane {
  Vector3D v;
  float d;
} PlaneT;

void PlaneNormalize(PlaneT *plane);
void PlaneFromPointAndVector(PlaneT *plane, Vector3D *point, Vector3D *vector);
float PointDistanceFromPlane(Vector3D *point, PlaneT *plane);

#endif
