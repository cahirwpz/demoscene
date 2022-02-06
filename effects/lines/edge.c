#include <stdio.h>

static inline short abs(short x) {
  return x >= 0 ? x : -x;
}

static void putpixel(int ptr, short xi, int stride) {
  /* *ptr |= (1 << xi); */

  int wb = 1 << (xi & 7);
  int offset = ptr % stride;
  int y = ptr / stride;
  int x = (~xi & 7) + offset * 8;
  printf("%2d %2d (%2d:%02x)\n", x, y, offset, wb);
}

void edge(int stride, short xs, short ys, short xe, short ye) {
  printf("\nedge: (%d, %d) -> (%d, %d)\n", xs, ys, xe, ye);

  if (ys > ye) {
    short xt = xs; xs = xe; xe = xt;
    short yt = ys; ys = ye; ye = yt;
  }

  short dx = xe - xs;
  short dy = ye - ys;

  if (dy == 0)
    return;

  int ptr = stride * ys + xs / 8;

  short n = dy;
  short di, df;
  short adx = abs(dx);

  if (adx < dy) {
    di = 0;
    df = adx;
  } else {
    di = adx / dy;
    df = adx % dy;
  }

  int dp = stride;
  short xi = ~xs & 7;
  short xf = -dy;

  if (dx >= 0) {
    dp += di / 8;
    di = (di & 7);

    do {
      putpixel(ptr, xi, stride);

      ptr += dp;

      xf += df;
      if (xf >= 0) {
        xi--;
        xf -= dy;
      }

      xi -= di;
      if (xi < 0) {
        ptr++;
        xi += 8;
      }
    } while (--n);
  } else {
    dp -= di / 8;
    di = -(di & 7);
    xi -= 8;

    do {
      putpixel(ptr, xi, stride);

      ptr += dp;

      xf += df;
      if (xf >= 0) {
        xi++;
        xf -= dy;
      }

      xi -= di;
      if (xi >= 0) {
        ptr--;
        xi -= 8;
      }
    } while (--n);
  }
}

int main(void) {
  edge(320/8, 8, 0, 56, 32);
  edge(320/8, 56, 0, 8, 32);
  return 0;
}
