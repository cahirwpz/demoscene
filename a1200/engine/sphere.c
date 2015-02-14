#include "engine/sphere.h"

float PointDistanceFromSphere(SphereT *sphere, Vector3D *point) {
  return V3D_Distance(&sphere->center, point) - sphere->radius;
}
