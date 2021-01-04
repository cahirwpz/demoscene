#include <3d.h>
#include <fx.h>

void Scale3D(Matrix3D *M, short sx, short sy, short sz) {
  short *m = &M->m00;
  short r;

  r = normfx((*m) * sx); *m++ = r;
  r = normfx((*m) * sy); *m++ = r;
  r = normfx((*m) * sz); *m++ = r;
  m++;

  r = normfx((*m) * sx); *m++ = r;
  r = normfx((*m) * sy); *m++ = r;
  r = normfx((*m) * sz); *m++ = r;
  m++;

  r = normfx((*m) * sx); *m++ = r;
  r = normfx((*m) * sy); *m++ = r;
  r = normfx((*m) * sz); *m++ = r;
}
