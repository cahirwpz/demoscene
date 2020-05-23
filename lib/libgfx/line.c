#include "line.h"

static struct {
  u_char *pixels;
  int stride;
} line;

__regargs void CpuLineSetup(const BitmapT *bitmap, u_short plane) {
  line.pixels = bitmap->planes[plane];
  line.stride = bitmap->bytesPerRow;
}

void CpuLine(short xs asm("d0"), short ys asm("d1"), short xe asm("d2"), short ye asm("d3")) {
  register u_char *pixels asm("a0") = line.pixels;
  register int stride asm("a1") = line.stride;
  u_char color;
  short dx, dy;

  if (ys > ye) {
    swapr(xs, xe);
    swapr(ys, ye);
  }

  dx = abs(xe - xs);
  dy = ye - ys;

  pixels += ys * (short)stride;
  pixels += (int)xs >> 3;

  color = 0x80 >> (xs & 7);

  if (dx < dy) {
    short dg2 = 2 * dx;
    short dg = dg2 - dy;
    short dg1 = dg - dy;

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
    short dg2 = 2 * dy;
    short dg = dg2 - dx;
    short dg1 = dg - dx;

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

void CpuEdge(short x1 asm("d0"), short y1 asm("d1"), short x2 asm("d2"), short y2 asm("d3")) {
  register u_char *pixels asm("a0") = line.pixels;
  register int stride asm("a1") = line.stride;

  if (y1 == y2)
    return;

  {
    short dy = y2 - y1;
    short dx = x2 - x1;
    u_char color = 0x80 >> (x1 & 7);

    pixels += (int)((short)y1 * (short)stride);
    pixels += x1 >> 3;

    if (dx < 0)
      dx = -dx;

    if (dx <= dy) {
      short err = dy / 2;
      short n = dy - 1;

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
      short err = dx / 2; /* Makes line symmetric on both ends... I guess */
      short n = dx - 1;

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
