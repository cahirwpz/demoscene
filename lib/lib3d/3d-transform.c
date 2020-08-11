#include <string.h>
#include "3d.h"
#include "fx.h"

void LoadIdentity3D(Matrix3D *M) {
  memset(M, 0, sizeof(Matrix3D));
  M->m00 = fx12f(1.0);
  M->m11 = fx12f(1.0);
  M->m22 = fx12f(1.0);
}

void Translate3D(Matrix3D *M, short x, short y, short z) {
  M->x += x;
  M->y += y;
  M->z += z;
}

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
void LoadRotate3D(Matrix3D *M, short ax, short ay, short az) {
  short sinX = SIN(ax);
  short cosX = COS(ax);
  short sinY = SIN(ay);
  short cosY = COS(ay);
  short sinZ = SIN(az);
  short cosZ = COS(az);
  
  short tmp0 = normfx(sinY * cosZ);
  short tmp1 = normfx(sinY * sinZ);

  short *m = &M->m00;

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
void LoadReverseRotate3D(Matrix3D *M, short ax, short ay, short az) {
  short sinX = SIN(ax);
  short cosX = COS(ax);
  short sinY = SIN(ay);
  short cosY = COS(ay);
  short sinZ = SIN(az);
  short cosZ = COS(az);

  short tmp0 = normfx(sinX * sinY);
  short tmp1 = normfx(cosX * sinY);

  short *m = &M->m00;

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
  int t0 = (*a++) * (*b0++);  \
  int t1 = (*a++) * (*b1++);  \
  int t2 = (*a++) * (*b2++);  \
  *d++ = normfx(t0 + t1 + t2); \
}

void Compose3D(Matrix3D *md, Matrix3D *ma, Matrix3D *mb) {
  short *a = &ma->m00;
  short *d = &md->m00;

  {
    short *b0 = &mb->m00;
    short *b1 = &mb->m10;
    short *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b0);
  }

  {
    short *b0 = &mb->m00;
    short *b1 = &mb->m10;
    short *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b1);
  }

  {
    short *b0 = &mb->m00;
    short *b1 = &mb->m10;
    short *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b2);
  }
}

#define MULVERTEX() {                 \
  short v0 = (*v++);                   \
  short v1 = (*v++);                   \
  short v2 = (*v++);                   \
  short t3 = (*v++);                   \
  int t0 = v0 * x;                   \
  int t1 = v1 * y;                   \
  int t2 = v2 * z;                   \
  *dst++ = normfx(t0 + t1 + t2) + t3; \
}

void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, short n) {
  short *src = (short *)in;
  short *dst = (short *)out;

  while (--n >= 0) {
    short *v = (short *)M;
    short x = *src++;
    short y = *src++;
    short z = *src++;

    MULVERTEX();
    MULVERTEX();
    MULVERTEX();

    src++; dst++;
  }
}
