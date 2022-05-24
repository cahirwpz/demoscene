#include <3d.h>
#include <fx.h>

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
