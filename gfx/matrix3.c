#include <math.h>

#include "system/memory.h"
#include "gfx/matrix3.h"

#define MX3_FOREACH(I, J)         \
  for ((I) = 0; (I) < 3; (I)++)   \
    for ((J) = 0; (J) < 3; (J)++)

mx3_t* mx3_new() {
  mx3_t *m = NEW_S(mx3_t);

  if (m)
    mx3_load_identity(m);

  return m;
}

void mx3_delete(mx3_t* m) {
  DELETE(m);
}

void mx3_mul(mx3_t *c, mx3_t *a, mx3_t *b) {
  int i, j, k;

  MX3_FOREACH(i, j) {
    MX3(c,i,j) = 0.0f;

    for (k = 0; k < 3; k++)
      MX3(c,i,j) += MX3(a,i,k) * MX3(b,k,j);
  }
}

void mx3_transpose(mx3_t *a, mx3_t *b) {
  int i, j; 

  MX3_FOREACH(i, j)
    MX3(a,i,j) = MX3(b,j,i);
}

void mx3_load_identity(mx3_t *m) {
  int i, j; 

  MX3_FOREACH(i, j)
    MX3(m,i,j) = (i == j) ? 1.0f : 0.0f;
}

void mx3_load_rotation(mx3_t *m, float angle) {
  mx3_load_identity(m);

  MX3(m,0,0) = cos(angle);
  MX3(m,1,0) = -sin(angle);
  MX3(m,0,1) = sin(angle);
  MX3(m,1,1) = cos(angle);
}

void mx3_load_scaling(mx3_t *m, float sx, float sy) {
  mx3_load_identity(m);

  MX3(m,0,0) = sx;
  MX3(m,1,1) = sy;
}

void mx3_load_translation(mx3_t *m, float tx, float ty) {
  mx3_load_identity(m);

  MX3(m,2,0) = tx;
  MX3(m,2,1) = ty;
}

void mx3_transform(mx3_t *m, point_t *from, point_t *to, int n) {
  int i;

  for (i = 0; i < n; i++) {
    float x = from[i].x;
    float y = from[i].y;

    to[i].x = (int16_t)(MX3(m,0,0) * x + MX3(m,1,0) * y + MX3(m,2,0));
    to[i].y = (int16_t)(MX3(m,0,1) * x + MX3(m,1,1) * y + MX3(m,2,1));
  }
}
