#include "system/memory.h"
#include "gfx/matrix3d.h"

Matrix3D *NewMatrix3D() {
  return NEW_SZ(Matrix3D);
}

void DeleteMatrix3D(Matrix3D *matrix) {
  DELETE(matrix);
}
