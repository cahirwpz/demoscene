#include <3d.h>
#include <fx.h>

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
