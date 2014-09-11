#include "line.h"

static struct {
  UBYTE *pixels;
  LONG stride;
} line;

__regargs void CpuLineSetup(BitmapT *bitmap, UWORD plane) {
  line.pixels = bitmap->planes[plane];
  line.stride = bitmap->bytesPerRow;
}

__regargs void CpuLine(WORD xs, WORD ys, WORD xe, WORD ye) {
  UBYTE *pixels = line.pixels;
  LONG stride = line.stride;
  UBYTE color;
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
