#include <3d.h>

void Translate3D(Matrix3D *M, short x, short y, short z) {
  M->x += x;
  M->y += y;
  M->z += z;
}
