#include "3d.h"
#include "fx.h"

__regargs void LoadIdentity3D(Matrix3D *M) {
  M->m00 = fx12f(1.0);
  M->m01 = 0;
  M->m02 = 0;
  M->x = 0;

  M->m10 = 0;
  M->m11 = fx12f(1.0);
  M->m12 = 0;
  M->y = 0;

  M->m10 = 0;
  M->m11 = 0;
  M->m12 = fx12f(1.0);
  M->y = 0;
}

__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z) {
  M->x += x;
  M->y += y;
  M->z += z;
}

__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz) {
  M->m00 = normfx(M->m00 * sx);
  M->m01 = normfx(M->m01 * sy);
  M->m02 = normfx(M->m02 * sz);

  M->m10 = normfx(M->m10 * sx);
  M->m11 = normfx(M->m11 * sy);
  M->m12 = normfx(M->m12 * sz);

  M->m20 = normfx(M->m20 * sx);
  M->m21 = normfx(M->m21 * sy);
  M->m22 = normfx(M->m22 * sz);
}

#define MULROW() {            \
  WORD *c = a;                \
  LONG t0 = (*c++) * (*b++);  \
  LONG t1 = (*c++) * (*b++);  \
  LONG t2 = (*c++) * (*b++);  \
  *m++ = normfx(t0 + t1 + t2); \
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
  WORD sinX = SIN(ax);
  WORD cosX = COS(ax);
  WORD sinY = SIN(ay);
  WORD cosY = COS(ay);
  WORD sinZ = SIN(az);
  WORD cosZ = COS(az);

  WORD tmp0 = normfx(sinX * sinY);
  WORD tmp1 = normfx(cosX * sinY);

  M->m00 = normfx(cosY * cosZ);
  M->m01 = normfx(cosY * sinZ);
  M->m02 = -sinY;
  M->x = 0;

  M->m10 = normfx(tmp0 * cosZ - cosX * sinZ);
  M->m11 = normfx(tmp0 * sinZ + cosX * cosZ);
  M->m12 = normfx(sinX * cosY);
  M->y = 0;

  M->m20 = normfx(tmp1 * cosZ + sinX * sinZ);
  M->m21 = normfx(tmp1 * sinZ - sinX * cosZ);
  M->m22 = normfx(cosX * cosY);
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
    WORD sinX = SIN(ax);
    WORD cosX = COS(ax);
    WORD sinY = SIN(ay);
    WORD cosY = COS(ay);
    WORD sinZ = SIN(az);
    WORD cosZ = COS(az);

    WORD tmp0 = normfx(sinX * sinY);
    WORD tmp1 = normfx(cosX * sinY);

    mb[0] = normfx(cosY * cosZ);
    mb[3] = normfx(cosY * sinZ);
    mb[6] = -sinY;
    mb[1] = normfx(tmp0 * cosZ - cosX * sinZ);
    mb[4] = normfx(tmp0 * sinZ + cosX * cosZ);
    mb[7] = normfx(sinX * cosY);
    mb[2] = normfx(tmp1 * cosZ + sinX * sinZ);
    mb[5] = normfx(tmp1 * sinZ - sinX * cosZ);
    mb[8] = normfx(cosX * cosY);
  }

  Compose3D(M, ma, mb);
}

#define MULVERTEX() {                 \
  LONG t0 = (*v++) * x;               \
  LONG t1 = (*v++) * y;               \
  LONG t2 = (*v++) * z;               \
  LONG t3 = (*v++);                   \
  *dst++ = normfx(t0 + t1 + t2) + t3; \
}

__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, WORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;

  while (--n >= 0) {
    WORD *v = (WORD *)M;
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;

    MULVERTEX();
    MULVERTEX();
    MULVERTEX();
  }
}
