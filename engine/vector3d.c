#include <math.h>
#include <string.h>

#include "std/fastmath.h"
#include "engine/vector3d.h"

void V3D_Add(Vector3D *d, Vector3D *a, Vector3D *b) {
  d->x = a->x + b->x;
  d->y = a->y + b->y;
  d->z = a->z + b->z;
}

void V3D_Sub(Vector3D *d, Vector3D *a, Vector3D *b) {
  d->x = a->x - b->x;
  d->y = a->y - b->y;
  d->z = a->z - b->z;
}

void V3D_Cross(Vector3D *d, Vector3D *a, Vector3D *b) {
  Vector3D w;

  w.x = a->y * b->z - b->y * a->z;
  w.y = a->z * b->x - b->z * a->x;
  w.z = a->x * b->y - b->x * a->y;

  memcpy(d, &w, sizeof(w));
}

float V3D_Dot(Vector3D *a, Vector3D *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

float V3D_Length(Vector3D *a) {
  return sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
}

float V3D_Distance(Vector3D *a, Vector3D *b) {
  Vector3D d;
  V3D_Sub(&d, a, b);
  return V3D_Length(&d);
}

void V3D_Scale(Vector3D *d, Vector3D *a, float s) {
  d->x = a->x * s;
  d->y = a->y * s;
  d->z = a->z * s;
}

void V3D_Normalize(Vector3D *d, Vector3D *a, float l) {
  float al = a->x * a->x + a->y * a->y + a->z * a->z;

  V3D_Scale(d, a, l * FastInvSqrt(al));
}

void V3D_NormalizeToUnit(Vector3D *d, Vector3D *a) {
  float al = a->x * a->x + a->y * a->y + a->z * a->z;

  V3D_Scale(d, a, FastInvSqrt(al));
}
