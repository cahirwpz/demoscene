#include <math.h>

#include "gfx/matrix2d.h"

#define M2D_FOREACH(I, J)         \
  for ((I) = 0; (I) < 3; (I)++)   \
    for ((J) = 0; (J) < 3; (J)++)

#define M(A,I,J) (*A)[I][J]

void M2D_Multiply(Matrix2D *d, Matrix2D *a, Matrix2D *b) {
  int i, j, k;

  M2D_FOREACH(i, j) {
    M(d,i,j) = 0.0f;

    for (k = 0; k < 3; k++)
      M(d,i,j) += M(a,i,k) * M(b,k,j);
  }
}

void M2D_Transpose(Matrix2D *d, Matrix2D *a) {
  int i, j; 

  M2D_FOREACH(i, j)
    M(d,i,j) = M(a,j,i);
}

void M2D_LoadIdentity(Matrix2D *d) {
  int i, j; 

  M2D_FOREACH(i, j)
    M(d,i,j) = (i == j) ? 1.0f : 0.0f;
}

void M2D_LoadRotation(Matrix2D *d, float angle) {
  M2D_LoadIdentity(d);

  angle *= 3.14159265 / 180.0;

  M(d,0,0) = cos(angle);
  M(d,1,0) = -sin(angle);
  M(d,0,1) = sin(angle);
  M(d,1,1) = cos(angle);
}

void M2D_LoadScaling(Matrix2D *d, float sx, float sy) {
  M2D_LoadIdentity(d);

  M(d,0,0) = sx;
  M(d,1,1) = sy;
}

void M2D_LoadTranslation(Matrix2D *d, float tx, float ty) {
  M2D_LoadIdentity(d);

  M(d,2,0) = tx;
  M(d,2,1) = ty;
}

void M2D_Transform(PointT *dst, PointT *src, int n, Matrix2D *m) {
  int i;

  for (i = 0; i < n; i++) {
    float x = src[i].x;
    float y = src[i].y;

    dst[i].x = (int16_t)(M(m,0,0) * x + M(m,1,0) * y + M(m,2,0));
    dst[i].y = (int16_t)(M(m,0,1) * x + M(m,1,1) * y + M(m,2,1));
  }
}
