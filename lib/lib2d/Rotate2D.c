#include <2d.h>
#include <fx.h>

void Rotate2D(Matrix2D *M, short a) {
  short sin = SIN(a);
  short cos = COS(a);
  short m00 = M->m00;
  short m01 = M->m01;
  short m10 = M->m10;
  short m11 = M->m11;

  M->m00 = normfx(m00 * cos - m01 * sin);
  M->m01 = normfx(m00 * sin + m01 * cos);
  M->m10 = normfx(m10 * cos - m11 * sin);
  M->m11 = normfx(m10 * sin + m11 * cos);
}
