#include <string.h>
#include <3d.h>
#include <fx.h>

void LoadIdentity3D(Matrix3D *M) {
  memset(M, 0, sizeof(Matrix3D));
  M->m00 = fx12f(1.0);
  M->m11 = fx12f(1.0);
  M->m22 = fx12f(1.0);
}
