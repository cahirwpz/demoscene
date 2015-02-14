#ifndef __ENGINE_PLANE_H__
#define __ENGINE_PLANE_H__

#include "engine/vector3d.h"

typedef struct Plane {
  Vector3D v;
  float d;
} PlaneT;

void PlaneNormalize(PlaneT *plane);
void PlaneFromPointAndVector(PlaneT *plane, Vector3D *point, Vector3D *vector);
float PointDistanceFromPlane(PlaneT *plane, Vector3D *point);

#endif
