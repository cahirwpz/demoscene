#include "line.h"

static struct {
  UBYTE *pixels;
  LONG stride;
} line;

__regargs void CpuLineSetup(BitmapT *bitmap, UWORD plane) {
  line.pixels = bitmap->planes[plane];
  line.stride = bitmap->bytesPerRow;
}

void CpuLine(WORD xs asm("d0"), WORD ys asm("d1"), WORD xe asm("d2"), WORD ye asm("d3")) {
  register UBYTE *pixels asm("a0") = line.pixels;
  register LONG stride asm("a1") = line.stride;
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

void CpuEdge(WORD x1 asm("d0"), WORD y1 asm("d1"), WORD x2 asm("d2"), WORD y2 asm("d3")) {
  register UBYTE *pixels asm("a0") = line.pixels;
  register LONG stride asm("a1") = line.stride;

  if (y1 == y2)
    return;

  {
    WORD dy = y2 - y1;
    WORD dx = x2 - x1;
    UBYTE color = 0x80 >> (x1 & 7);

    pixels += (LONG)((WORD)y1 * (WORD)stride);
    pixels += x1 >> 3;

    if (dx < 0)
      dx = -dx;

    if (dx <= dy) {
      WORD err = dy / 2;
      WORD n = dy - 1;

      if (x1 <= x2) {
        do {
          *pixels ^= color;
          pixels += stride;
          err += dx;
          if (err >= dy) {
            asm("rorb  #1,%0\n"
                "bccs  . +4\n"
                "addql #1,%1\n"
                : "+d" (color), "+a" (pixels));
            err -= dy;
          }
        } while (--n != -1);
      } else {
        do {
          *pixels ^= color;
          pixels += stride;
          err += dx;
          if (err >= dy) {
            asm("rolb  #1,%0\n"
                "bccs  . +4\n"
                "subql #1,%1\n"
                : "+d" (color), "+a" (pixels));
            err -= dy;
          }
        } while (--n != -1);
      } 
    } else {
      WORD err = dx / 2; /* Makes line symmetric on both ends... I guess */
      WORD n = dx - 1;

      if (x1 <= x2) {
        do {
          err += dy;
          if (err >= dx) {
            *pixels ^= color;
            pixels += stride;
            err -= dx;
          }
          asm("rorb  #1,%0\n"
              "bccs  . +4\n"
              "addql #1,%1\n"
              : "+d" (color), "+a" (pixels));
        } while (--n != -1);
      } else {
        do {
          err += dy;
          if (err >= dx) {
            *pixels ^= color;
            pixels += stride;
            err -= dx;
          }
          asm("rolb  #1,%0\n"
              "bccs  . +4\n"
              "subql #1,%1\n"
              : "+d" (color), "+a" (pixels));
        } while (--n != -1);
      }
    }
  }
}
