#include "3d.h"

__regargs void LoadIdentity3D(Matrix3D *M) {
  M->m00 = 1 << 8;
  M->m01 = 0;
  M->m02 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = 1 << 8;
  M->m12 = 0;
  M->y = 0;

  M->m10 = 0;
  M->m11 = 0;
  M->m12 = 1 << 8;
  M->y = 0;
}

__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z) {
  M->x += x;
  M->y += y;
  M->z += z;
}

__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz) {
  M->m00 = (M->m00 * sx) >> 8;
  M->m01 = (M->m01 * sy) >> 8;
  M->m02 = (M->m02 * sz) >> 8;

  M->m10 = (M->m10 * sx) >> 8;
  M->m11 = (M->m11 * sy) >> 8;
  M->m12 = (M->m12 * sz) >> 8;

  M->m20 = (M->m20 * sx) >> 8;
  M->m21 = (M->m21 * sy) >> 8;
  M->m22 = (M->m22 * sz) >> 8;
}

#define MULROW() {            \
  WORD *c = a;                \
  LONG t0 = (*c++) * (*b++);  \
  LONG t1 = (*c++) * (*b++);  \
  LONG t2 = (*c++) * (*b++);  \
  *m++ = (t0 + t1 + t2) >> 8; \
}

/* Assumes "b" is a transposed 3x3 matrix! */
static __regargs void Compose3D(Matrix3D *d, WORD a[9], WORD b[9]) {
  WORD *m = &d->m00;

  pushl(b);
  MULROW();
  MULROW();
  MULROW();
  popl(b);
  a += 3;
  m++;

  pushl(b);
  MULROW();
  MULROW();
  MULROW();
  popl(b);
  a += 3;
  m++;

  MULROW();
  MULROW();
  MULROW();
}

__regargs void LoadRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  WORD sinX = sincos[ax & 0x1ff].sin;
  WORD cosX = sincos[ax & 0x1ff].cos;
  WORD sinY = sincos[ay & 0x1ff].sin;
  WORD cosY = sincos[ay & 0x1ff].cos;
  WORD sinZ = sincos[az & 0x1ff].sin;
  WORD cosZ = sincos[az & 0x1ff].cos;

  WORD tmp0 = (sinX * sinY) >> 8;
  WORD tmp1 = (cosX * sinY) >> 8;

  M->m00 = (cosY * cosZ) >> 8;
  M->m01 = (cosY * sinZ) >> 8;
  M->m02 = -sinY;
  M->x = 0;

  M->m10 = (tmp0 * cosZ - cosX * sinZ) >> 8;
  M->m11 = (tmp0 * sinZ + cosX * cosZ) >> 8;
  M->m12 = (sinX * cosY) >> 8;
  M->y = 0;

  M->m20 = (tmp1 * cosZ + sinX * sinZ) >> 8;
  M->m21 = (tmp1 * sinZ - sinX * cosZ) >> 8;
  M->m22 = (cosX * cosY) >> 8;
  M->z = 0;
}

__regargs void Rotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  WORD ma[9], mb[9];

  ma[0] = M->m00;
  ma[1] = M->m01;
  ma[2] = M->m02;
  ma[3] = M->m10;
  ma[4] = M->m11;
  ma[5] = M->m12;
  ma[6] = M->m20;
  ma[7] = M->m21;
  ma[8] = M->m22;

  {
    WORD sinX = sincos[ax & 0x1ff].sin;
    WORD cosX = sincos[ax & 0x1ff].cos;
    WORD sinY = sincos[ay & 0x1ff].sin;
    WORD cosY = sincos[ay & 0x1ff].cos;
    WORD sinZ = sincos[az & 0x1ff].sin;
    WORD cosZ = sincos[az & 0x1ff].cos;

    WORD tmp0 = (sinX * sinY) >> 8;
    WORD tmp1 = (cosX * sinY) >> 8;

    mb[0] = (cosY * cosZ) >> 8;
    mb[3] = (cosY * sinZ) >> 8;
    mb[6] = -sinY;
    mb[1] = (tmp0 * cosZ - cosX * sinZ) >> 8;
    mb[4] = (tmp0 * sinZ + cosX * cosZ) >> 8;
    mb[7] = (sinX * cosY) >> 8;
    mb[2] = (tmp1 * cosZ + sinX * sinZ) >> 8;
    mb[5] = (tmp1 * sinZ - sinX * cosZ) >> 8;
    mb[8] = (cosX * cosY) >> 8;
  }

  Compose3D(M, ma, mb);
}

#define MULVERTEX() {                  \
  LONG t0 = (*v++) * x;                \
  LONG t1 = (*v++) * y;                \
  LONG t2 = (*v++) * z;                \
  LONG t3 = (*v++);                    \
  *dst++ = ((t0 + t1 + t2) >> 8) + t3; \
}

__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, UWORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;
  WORD *v = (WORD *)M;

  while (n--) {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;

    pushl(v);
    MULVERTEX();
    MULVERTEX();
    MULVERTEX();
    popl(v);
  }
}

__regargs void PointsInsideFrustum(Point3D *in, UBYTE *flags, UWORD n,
                                   WORD near, WORD far)
{
  WORD *src = (WORD *)in;

  while (n--) {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;
    UBYTE f = 0;

    if (x < -z)
      f |= PF_LEFT;
    if (x >= z)
      f |= PF_RIGHT;
    if (y < -z)
      f |= PF_TOP;
    if (y >= z)
      f |= PF_BOTTOM;
    if (y < near)
      f |= PF_NEAR;
    if (z >= far)
      f |= PF_FAR;

    *flags++ = f;
  }
}
