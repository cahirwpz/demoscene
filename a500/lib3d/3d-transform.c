#include "3d.h"
#include "fx.h"

__regargs void LoadIdentity3D(Matrix3D *M) {
  memset(M, 0, sizeof(Matrix3D));
  M->m00 = fx12f(1.0);
  M->m11 = fx12f(1.0);
  M->m22 = fx12f(1.0);
}

__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z) {
  M->x += x;
  M->y += y;
  M->z += z;
}

__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz) {
  WORD *m = &M->m00;
  WORD r;

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

/*
 * Rx(x) = {{1,0,0},{0,cos(x),-sin(x)},{0,sin(x),cos(x)}}
 * Ry(y) = {{cos(y),0,sin(y)},{0,1,0},{-sin(y),0,cos(y)}}
 * Rz(z) = {{cos(z),-sin(z),0},{sin(z),cos(z),0},{0,0,1}}
 *
 * Rx(x) * Ry(y) * Rz(z) :
 *
 * [                          cos(y)*cos(z) |                         -cos(y)*sin(z) |         sin(y) ]
 * [ cos(x)*sin(z) + sin(x)*(sin(y)*cos(z)) | cos(x)*cos(z) - sin(x)*(sin(y)*sin(z)) | -sin(x)*cos(y) ]
 * [ sin(x)*sin(z) - cos(x)*(sin(y)*cos(z)) | sin(x)*cos(z) + cos(x)*(sin(y)*sin(z)) |  cos(x)*cos(y) ]
 */
__regargs void LoadRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  WORD sinX = SIN(ax);
  WORD cosX = COS(ax);
  WORD sinY = SIN(ay);
  WORD cosY = COS(ay);
  WORD sinZ = SIN(az);
  WORD cosZ = COS(az);
  
  WORD tmp0 = normfx(sinY * cosZ);
  WORD tmp1 = normfx(sinY * sinZ);

  WORD *m = &M->m00;

  *m++ = normfx(cosY * cosZ);
  *m++ = - normfx(cosY * sinZ);
  *m++ = sinY;
  *m++ = 0;

  *m++ = normfx(cosX * sinZ + sinX * tmp0);
  *m++ = normfx(cosX * cosZ - sinX * tmp1);
  *m++ = - normfx(sinX * cosY);
  *m++ = 0;

  *m++ = normfx(sinX * sinZ - cosX * tmp0);
  *m++ = normfx(sinX * cosZ + cosX * tmp1);
  *m++ = normfx(cosX * cosY);
  *m++ = 0;
}

/*
 * Rz(z) * Ry(y) * Rx(x) :
 *
 * [ cos(y)*cos(z) | (sin(x)*sin(y))*cos(z) - cos(x)*sin(z) | (cos(x)*sin(y))*cos(z) + sin(x)*sin(z) ]
 * [ cos(y)*sin(z) | (sin(x)*sin(y))*sin(z) + cos(x)*cos(z) | (cos(x)*sin(y))*sin(z) - sin(x)*cos(z) ]
 * [       -sin(y) |                          sin(x)*cos(y) |                          cos(x)*cos(y) ]
 */
__regargs void LoadReverseRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az) {
  WORD sinX = SIN(ax);
  WORD cosX = COS(ax);
  WORD sinY = SIN(ay);
  WORD cosY = COS(ay);
  WORD sinZ = SIN(az);
  WORD cosZ = COS(az);

  WORD tmp0 = normfx(sinX * sinY);
  WORD tmp1 = normfx(cosX * sinY);

  WORD *m = &M->m00;

  *m++ = normfx(cosY * cosZ);
  *m++ = normfx(tmp0 * cosZ - cosX * sinZ);
  *m++ = normfx(tmp1 * cosZ + sinX * sinZ);
  *m++ = 0;

  *m++ = normfx(cosY * sinZ);
  *m++ = normfx(tmp0 * sinZ + cosX * cosZ);
  *m++ = normfx(tmp1 * sinZ - sinX * cosZ);
  *m++ = 0;

  *m++ = -sinY;
  *m++ = normfx(sinX * cosY);
  *m++ = normfx(cosX * cosY);
  *m++ = 0;
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

#define MULVERTEX() {                 \
  WORD v0 = (*v++);                   \
  WORD v1 = (*v++);                   \
  WORD v2 = (*v++);                   \
  WORD t3 = (*v++);                   \
  LONG t0 = v0 * x;                   \
  LONG t1 = v1 * y;                   \
  LONG t2 = v2 * z;                   \
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

    src++; dst++;
  }
}
