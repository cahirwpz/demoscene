#include "line.h"

__regargs void CpuLine(BitmapT *bitmap, UWORD plane,
                       UWORD xs, UWORD ys, UWORD xe, UWORD ye)
{
  UBYTE *pixels = bitmap->planes[plane];
  ULONG stride = bitmap->width / 8;
  UBYTE color = 0x80;
  WORD dx, dy;

  if (ys > ye) {
    asm("exg %0,%1" : "+r" (xs), "+r" (xe));
    asm("exg %0,%1" : "+r" (ys), "+r" (ye));
  }

  dx = abs(xe - xs);
  dy = ye - ys;

  pixels += (ys * bitmap->width + xs) / 8;
  color >>= (xs & 7);

  if (dx < dy) {
    WORD dg1 = (dx - dy) << 1;
    WORD dg2 = dx << 1;
    WORD dg = dg2 - dy;

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
    WORD dg1 = (dy - dx) << 1;
    WORD dg2 = dy << 1;
    WORD dg = dg2 - dx;

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
