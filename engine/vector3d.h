#ifndef __ENGINE_VECTOR3D_H__
#define __ENGINE_VECTOR3D_H__

#include <math.h>
#include <string.h>

#include "std/fastmath.h"
#include "std/types.h"

typedef struct {
  float x, y, z;
} Vector3D;

static inline void
V3D_Add(Vector3D *d, const Vector3D *a, const Vector3D *b) {
  d->x = a->x + b->x;
  d->y = a->y + b->y;
  d->z = a->z + b->z;
}

static inline void
V3D_Sub(Vector3D *d, const Vector3D *a, const Vector3D *b) {
  d->x = a->x - b->x;
  d->y = a->y - b->y;
  d->z = a->z - b->z;
}

static inline void
V3D_Cross(Vector3D *d, const Vector3D *a, const Vector3D *b) {
  Vector3D w;

  w.x = a->y * b->z - b->y * a->z;
  w.y = a->z * b->x - b->z * a->x;
  w.z = a->x * b->y - b->x * a->y;

  d->x = w.x;
  d->y = w.y;
  d->z = w.z;
}

static inline float
V3D_Dot(const Vector3D *a, const Vector3D *b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline float
V3D_Length(const Vector3D *a) {
  return sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
}

static inline float
V3D_Distance(const Vector3D *a, const Vector3D *b) {
  Vector3D d;
  V3D_Sub(&d, a, b);
  return V3D_Length(&d);
}

static inline void
V3D_Scale(Vector3D *d, const Vector3D *a, const float s) {
  d->x = a->x * s;
  d->y = a->y * s;
  d->z = a->z * s;
}

static inline void
V3D_Normalize(Vector3D *d, const Vector3D *a, const float l) {
  float al = a->x * a->x + a->y * a->y + a->z * a->z;

  V3D_Scale(d, a, l * FastInvSqrt(al));
}

static inline void
V3D_NormalizeToUnit(Vector3D *d, const Vector3D *a) {
  float al = a->x * a->x + a->y * a->y + a->z * a->z;

  V3D_Scale(d, a, FastInvSqrt(al));
}

#endif
