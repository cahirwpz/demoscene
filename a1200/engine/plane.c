#include "engine/plane.h"

void PlaneNormalize(PlaneT *plane) {
  float factor = 1.0f / V3D_Length(&plane->v);

  V3D_Scale(&plane->v, &plane->v, factor);
  plane->d *= factor;
}

void PlaneFromPointAndVector(PlaneT *plane, Vector3D *point, Vector3D *vector) {
  V3D_Normalize(&plane->v, &plane->v, V3D_Length(vector));
  plane->d = -V3D_Dot(&plane->v, point);
}

float PointDistanceFromPlane(PlaneT *plane, Vector3D *point) {
  return V3D_Dot(&plane->v, point) + plane->d;
}
