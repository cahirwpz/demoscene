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

  M->m20 = 0;
  M->m21 = 0;
  M->m22 = fx12f(1.0);
  M->z = 0;
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

#define MULROW() {             \
  LONG t0 = (*a++) * (*b0++);  \
  LONG t1 = (*a++) * (*b1++);  \
  LONG t2 = (*a++) * (*b2++);  \
  *d++ = normfx(t0 + t1 + t2); \
}

__regargs void Compose3D(Matrix3D *md, Matrix3D *ma, Matrix3D *mb) {
  WORD *a = &ma->m00;
  WORD *d = &md->m00;

  {
    WORD *b0 = &mb->m00;
    WORD *b1 = &mb->m10;
    WORD *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b0);
  }

  {
    WORD *b0 = &mb->m00;
    WORD *b1 = &mb->m10;
    WORD *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b1);
  }

  {
    WORD *b0 = &mb->m00;
    WORD *b1 = &mb->m10;
    WORD *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b2);
  }
}

__regargs void Rotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  Matrix3D tmp, rot;

  LoadRotate3D(&rot, ax, ay, az);
  memcpy(&tmp, M, sizeof(Matrix3D));

  Compose3D(M, &tmp, &rot);
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
