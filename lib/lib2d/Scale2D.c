#include <2d.h>
#include <fx.h>

void Scale2D(Matrix2D *M, short sx, short sy) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
}
