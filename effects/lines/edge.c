#include <stdio.h>

static inline short abs(short x) {
  return x >= 0 ? x : -x;
}

void edge(short xs, short ys, short xe, short ye) {
  printf("\nedge: (%d, %d) -> (%d, %d)\n", xs, ys, xe, ye);

  if (ys > ye) {
    short xt = xs; xs = xe; xe = xt;
    short yt = ys; ys = ye; ye = yt;
  }

  short dx = xe - xs;
  short dy = ye - ys;

  if (dy == 0)
    return;

  short di = abs(dx) / dy;
  short df = abs(dx) % dy;
  short si;

  if (dx >= 0) {
    si = 1;
  } else {
    si = -1;
    di = -di;
  }

#if 0
  short xi = xs;
  short xf = 0;
  short n = dy;

  do {
    short wi = xi >> 3;
    int wb = 1 << (~xi & 7);
    printf("%2d %2d (%2d:%02x)\n", xi, ys, wi, wb);

    ys += 1;
    xi += di;
    xf += df;
    if (xf >= dy) {
      xf -= dy;
      xi += si;
    }
  } while (--n);
#else
  short n = dy;
  int x = (xs << 16) | 0;
  int d = (di << 16) | df;
  int s = (si << 16) - dy;

  do {
    short xi = x >> 16;
    short wi = xi >> 3;
    int wb = 1 << (~xi & 7);
    printf("%2d %2d (%2d:%02x)\n", xi, ys, wi, wb);

    ys += 1;
    x += d;
    if ((short)x >= dy)
      x += s;
  } while (--n);
#endif
}

int main(void) {
  edge(8, 0, 56, 32);
  edge(56, 0, 8, 32);
  return 0;
}
