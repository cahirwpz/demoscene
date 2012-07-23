#ifndef __ENGINE_SPHERE_H__
#define __ENGINE_SPHERE_H__

#include "engine/vector3d.h"

typedef struct Sphere {
  Vector3D center;
  float radius;
} SphereT;

float PointDistanceFromSphere(SphereT *sphere, Vector3D *point);

#endif
