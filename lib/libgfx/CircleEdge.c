#include <circle.h>

static inline void Plot(u_char *pixels, int pos) {
  bset(pixels + (pos >> 3), ~pos);
}

void CircleEdge(const BitmapT *bitmap, int plane, short x0, short y0, short r) {
  u_char *pixels = bitmap->planes[plane];
  int width = bitmap->bytesPerRow << 3;
  short f = 1 - r;
  short ddF_x = 0;
  short ddF_y = -(r << 1);
  register int x asm("d6") = 0;
  register int y asm("d7") = r;
  register int q0 asm("a3");
  register int q1 asm("a4");
  int q2;
  int q3;
  u_char dot;

  {
    int base = mul16(y0, width) + x0;
    int yr = mul16(r, width);

    q0 = base + yr;
    q1 = base - yr;
    q2 = base;
    q3 = base;
  }

  Plot(pixels, q2 + r);
  Plot(pixels, q3 - r);

  while (x < y) {
    if (f > 0) {
      y--;
      q0 -= width;
      q1 += width;
      ddF_y += 2;
      f += ddF_y;
      dot = -1;
    } else {
      dot = 0;
    }

    x++;
    q2 += width;
    q3 -= width;
    ddF_x += 2;
    f += ddF_x + 1;    

    if (dot) {
      Plot(pixels, q0 + x);
      Plot(pixels, q0 - x);
      Plot(pixels, q1 + x);
      Plot(pixels, q1 - x);
    }

    Plot(pixels, q2 + y);
    Plot(pixels, q2 - y);
    Plot(pixels, q3 + y);
    Plot(pixels, q3 - y);
  }
}
