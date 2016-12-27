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

__regargs void Translate2D(Matrix2D *M, WORD x, WORD y) {
  M->x += x;
  M->y += y;
}

__regargs void Scale2D(Matrix2D *M, WORD sx, WORD sy) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
}

__regargs void Rotate2D(Matrix2D *M, WORD a) {
  WORD sin = SIN(a);
  WORD cos = COS(a);
  WORD m00 = M->m00;
  WORD m01 = M->m01;
  WORD m10 = M->m10;
  WORD m11 = M->m11;

  M->m00 = normfx(m00 * cos - m01 * sin);
  M->m01 = normfx(m00 * sin + m01 * cos);
  M->m10 = normfx(m10 * cos - m11 * sin);
  M->m11 = normfx(m10 * sin + m11 * cos);
}

__regargs void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, WORD n) {
  WORD *dst = (WORD *)out;
  WORD *src = (WORD *)in;
  WORD *v = (WORD *)M;
  WORD m00 = *v++;
  WORD m01 = *v++;
  WORD mx  = *v++;
  WORD m10 = *v++;
  WORD m11 = *v++;
  WORD my  = *v++;

  while (--n >= 0) {
    WORD x = *src++;
    WORD y = *src++;

    *dst++ = normfx(m00 * x + m01 * y) + mx;
    *dst++ = normfx(m10 * x + m11 * y) + my;
  }
}
