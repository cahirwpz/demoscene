#include <3d.h>

void PointsInsideFrustum(Point3D *in, u_char *flags, u_short n) {
  short *src = (short *)in;

  while (n--) {
    short x = *src++;
    short y = *src++;
    short z = *src++;
    u_char f = 0;

    if (x < z)
      f |= PF_LEFT;
    if (x > -z)
      f |= PF_RIGHT;
    if (y < z)
      f |= PF_TOP;
    if (y > -z)
      f |= PF_BOTTOM;
    if (z > ClipFrustum.near)
      f |= PF_NEAR;
    if (z < ClipFrustum.far)
      f |= PF_FAR;

    *flags++ = f;
  }
}
