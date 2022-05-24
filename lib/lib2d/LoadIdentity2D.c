#include <2d.h>
#include <fx.h>

void LoadIdentity2D(Matrix2D *M) {
  M->m00 = fx12f(1.0);
  M->m01 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = fx12f(1.0);
  M->y = 0;
}
