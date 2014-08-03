#include "line.h"

__regargs void CpuLine(BitmapT *bitmap, ULONG plane, Line2D *line)
{
  UBYTE *pixels = bitmap->planes[plane];
  LONG stride = bitmap->width / 8;
  UBYTE color;
  WORD xs = line->x1;
  WORD ys = line->y1;
  WORD xe = line->x2;
  WORD ye = line->y2;
  WORD dx, dy;

  if (ys > ye) {
    swapr(xs, xe);
    swapr(ys, ye);
  }

  dx = abs(xe - xs);
  dy = ye - ys;

  pixels += ys * (WORD)stride;
  pixels += (LONG)xs >> 3;

  color = 0x80 >> (xs & 7);

  if (dx < dy) {
    WORD dg2 = 2 * dx;
    WORD dg = dg2 - dy;
    WORD dg1 = dg - dy;

    if (dy == 0)
      return;

    dy--;

    if (xe < xs) {
      do {
        *pixels |= color;

        pixels += stride;

        if (dg > 0) {
          asm("rolb  #1,%0\n"
              "bccs  . +4\n"
              "subql #1,%1\n" : "+r" (color), "+r" (pixels));
          dg += dg1;
        } else {
          dg += dg2;
        }
      } while (--dy != -1);
    } else {
      do {
        *pixels |= color;

        pixels += stride;

        if (dg > 0) {
          asm("rorb  #1,%0\n"
              "bccs  . +4\n"
              "addql #1,%1\n" : "+r" (color), "+r" (pixels));
          dg += dg1;
        } else {
          dg += dg2;
        }
      } while (--dy != -1);
    }
  } else {
    WORD dg2 = 2 * dy;
    WORD dg = dg2 - dx;
    WORD dg1 = dg - dx;

    if (dx == 0)
      return;

    dx--;

    if (xe < xs) {
      do {
        *pixels |= color;

        asm("rolb  #1,%0\n"
            "bccs  . +4\n"
            "subql #1,%1\n" : "+r" (color), "+r" (pixels));

        if (dg > 0) {
          pixels += stride;
          dg += dg1;
        } else {
          dg += dg2;
        }
      } while (--dx != -1);
    } else {
      do {
        *pixels |= color;

        asm("rorb  #1,%0\n"
            "bccs  . +4\n"
            "addql #1,%1\n" : "+r" (color), "+r" (pixels));

        if (dg > 0) {
          pixels += stride;
          dg += dg1;
        } else {
          dg += dg2;
        }
      } while (--dx != -1);
    }
  }
}
