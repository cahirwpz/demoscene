#include "circle.h"

__regargs void Circle(BitmapT *bitmap, LONG plane, WORD xc, WORD yc, WORD r) {
  LONG stride = bitmap->width / 8;

  UBYTE *pixels = bitmap->planes[plane] + yc * (WORD)stride;
  UBYTE *q0 = pixels;
  UBYTE *q1 = pixels;
  UBYTE *q2 = pixels - r * (WORD)stride;
  UBYTE *q3 = pixels + r * (WORD)stride;

  WORD x = -r;
  WORD y = 0;
  WORD err = 2 * (1 - r);

  WORD x0 = xc - x;
  WORD x1 = xc + x;
  WORD x2 = xc;
  WORD x3 = xc;

  do {
    /* (xc - x, yc + y) */
    bset(q0 + (x0 / 8), ~x0);
    
    /* (xc + x, yc - y) */
    bset(q1 + (x1 / 8), ~x1);

    /* (xc + y, yc + x) */
    bset(q2 + (x2 / 8), ~x2);
 
    /* (xc - y, yc - x) */
    bset(q3 + (x3 / 8), ~x3);

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
