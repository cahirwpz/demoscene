#include "2d.h"
#include "fx.h"

__regargs void LoadIdentity2D(Matrix2D *M) {
  M->m00 = fx12f(1.0);
  M->m01 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = fx12f(1.0);
  M->y = 0;
}

__regargs void Translate2D(Matrix2D *M, short x, short y) {
  M->x += x;
  M->y += y;
}

__regargs void Scale2D(Matrix2D *M, short sx, short sy) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
}

__regargs void Rotate2D(Matrix2D *M, short a) {
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

__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, short n) {
  short *dst = (short *)out;
  short *src = (short *)in;
  short *v = (short *)M;
  short m00 = *v++;
  short m01 = *v++;
  short mx  = *v++;
  short m10 = *v++;
  short m11 = *v++;
  short my  = *v++;

  while (--n >= 0) {
    short x = *src++;
    short y = *src++;

    *dst++ = normfx(m00 * x + m01 * y) + mx;
    *dst++ = normfx(m10 * x + m11 * y) + my;
  }
}
