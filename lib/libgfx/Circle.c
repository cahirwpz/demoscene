#include <circle.h>

void Circle(const BitmapT *bitmap, int plane, short xc, short yc, short r) {
  int stride = bitmap->bytesPerRow;

  u_char *pixels = bitmap->planes[plane] + yc * (short)stride;
  u_char *q0 = pixels;
  u_char *q1 = pixels;
  u_char *q2 = pixels - r * (short)stride;
  u_char *q3 = pixels + r * (short)stride;

  short x = -r;
  short y = 0;
  short err = 2 * (1 - r);

  short x0 = xc - x;
  short x1 = xc + x;
  short x2 = xc;
  short x3 = xc;

  do {
    /* (xc - x, yc + y) */
    bset(q0 + (x0 >> 3), ~x0);
    
    /* (xc + x, yc - y) */
    bset(q1 + (x1 >> 3), ~x1);

    /* (xc + y, yc + x) */
    bset(q2 + (x2 >> 3), ~x2);
 
    /* (xc - y, yc - x) */
    bset(q3 + (x3 >> 3), ~x3);

    if (err <= y) {
      q0 += stride;
      q1 -= stride;
      x2++;
      x3--;
      y++;
      err += y * 2 + 1;
    }
    if (err > x) {
      q2 += stride;
      q3 -= stride;
      x0--;
      x1++;
      x++;
      err += x * 2 + 1;
    }
  } while (x < 0);
}

void CircleEdge(const BitmapT *bitmap, int plane, short xc, short yc, short r) {
  int stride = bitmap->width >> 3;

  u_char *pixels = bitmap->planes[plane] + yc * (short)stride;
  u_char *q0 = pixels;
  u_char *q1 = pixels;
  u_char *q2 = pixels - r * (short)stride;
  u_char *q3 = pixels + r * (short)stride;

  short x = -r;
  short y = 0;
  short err = 2 * (1 - r);

  short x0 = xc - x;
  short x1 = xc + x;
  short x2 = xc;
  short x3 = xc;

  do {
    if (err <= y) {
      /* (xc - x, yc + y) */
      bset(q0 + (x0 >> 3), ~x0);
      /* (xc + x, yc - y) */
      bset(q1 + (x1 >> 3), ~x1);

      q0 += stride;
      q1 -= stride;
      x2++;
      x3--;

      y++;
      err += y * 2 + 1;
    }
    if (err > x) {
      q2 += stride;
      q3 -= stride;
      x0--;
      x1++;

      /* (xc + y, yc + x) */
      bset(q2 + (x2 >> 3), ~x2);

      /* (xc - y, yc - x) */
      bset(q3 + (x3 >> 3), ~x3);

      x++;
      err += x * 2 + 1;
    }
  } while (x < 0);
}
