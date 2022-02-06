#include <line.h>

static struct {
  u_char *pixels;
  int stride;
} line;

void CpuLineSetup(const BitmapT *bitmap, u_short plane) {
  line.pixels = bitmap->planes[plane];
  line.stride = bitmap->bytesPerRow;
}

void CpuLine(short xs asm("d0"), short ys asm("d1"),
             short xe asm("d2"), short ye asm("d3"))
{
  register void *pixels asm("a0") = line.pixels;
  short stride = line.stride;
  u_short color;
  short dx, dy;

  if (ys > ye) {
    swapr(xs, xe);
    swapr(ys, ye);
  }

  pixels += ys * stride;
  pixels += (xs >> 3) & -2;

  color = 0x8000 >> (xs & 15);

  dy = ye - ys;

  if (dy == 0)
    return;

  dx = abs(xe - xs);

  if (dx < dy) {
    short dg2 = 2 * dx;
    short dg = dg2 - dy;
    short dg1 = 2 * dy;

    dg -= dg2; /* precompensate for [dg += dg2] */
    dy--;

    if (xe < xs) {
      /* case 1 */
      do {
        *(u_short *)pixels |= color;

        dg += dg2;

        if (dg >= 0) {
          dg -= dg1;
          asm("rolw  #1,%0\n"
              "bccs  . +4\n"
              "subql #2,%1\n" : "+r" (color), "+r" (pixels));
        }

        pixels += stride;
      } while (--dy != -1);
    } else {
      /* case 2 */
      do {
        *(u_short *)pixels |= color;

        dg += dg2;

        if (dg >= 0) {
          asm("rorw  #1,%0\n"
              "bccs  . +4\n"
              "addql #2,%1\n" : "+r" (color), "+r" (pixels));
          dg -= dg1;
        }

        pixels += stride;
      } while (--dy != -1);
    }
  } else {
    short dg2 = 2 * dy;
    short dg = dg2 - dx;
    short dg1 = 2 * dx;

    dg -= dg2; /* precompensate for [dg += dg2] */
    dy--;

    if (xe < xs) {
      /* case 3 */
      do {
        *(u_short *)pixels |= color;

        dg += dg2;
        if (dg >= 0) {
          dg -= dg1;
          pixels += stride;
        }

        asm("rolw  #1,%0\n"
            "bccs  . +4\n"
            "subql #2,%1\n" : "+r" (color), "+r" (pixels));
      } while (--dx != -1);
    } else {
      /* case 4 */
      do {
        *(u_short *)pixels |= color;

        dg += dg2;
        if (dg >= 0) {
          dg -= dg1;
          pixels += stride;
        }

        asm("rorw  #1,%0\n"
            "bccs  . +4\n"
            "addql #2,%1\n" : "+r" (color), "+r" (pixels));
      } while (--dx != -1);
    }
  }
}
