#include <line.h>

static struct {
  u_char *pixels;
  int stride;
} edge;

void CpuEdgeSetup(const BitmapT *bitmap, u_short plane) {
  edge.pixels = bitmap->planes[plane];
  edge.stride = bitmap->bytesPerRow;
}

void CpuEdge(short x1 asm("d0"), short y1 asm("d1"), short x2 asm("d2"), short y2 asm("d3")) {
  register u_char *pixels asm("a0") = edge.pixels;
  register int stride asm("a1") = edge.stride;

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
