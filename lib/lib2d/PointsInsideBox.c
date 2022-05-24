#include <2d.h>

void PointsInsideBox(Point2D *in, u_char *flags, short n) {
  short *src = (short *)in;

  short minX = ClipWin.minX;
  short minY = ClipWin.minY;
  short maxX = ClipWin.maxX;
  short maxY = ClipWin.maxY;

  while (--n >= 0) {
    short x = *src++;
    short y = *src++;
    u_char f = 0;

    if (x < minX)
      f |= PF_LEFT;
    else if (x >= maxX)
      f |= PF_RIGHT;
    if (y < minY)
      f |= PF_TOP;
    else if (y >= maxY)
      f |= PF_BOTTOM;

    *flags++ = f;
  }
}
