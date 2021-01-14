#include <2d.h>
#include <fx.h>

void Transform2D(Matrix2D *M, Point2D *out, Point2D *in, short n) {
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
