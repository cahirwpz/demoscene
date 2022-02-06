#include <line.h>

static struct {
  u_char *pixels;
  int stride;
} edge;

void CpuEdgeSetup(const BitmapT *bitmap, u_short plane) {
  edge.pixels = bitmap->planes[plane];
  edge.stride = bitmap->bytesPerRow;
}

void CpuEdge(short xs asm("d0"), short ys asm("d1"),
             short xe asm("d2"), short ye asm("d3"))
{
  register u_char *pixels asm("a0") = edge.pixels;
  short stride = edge.stride;
  short dx, dy, di, df, n, xi, xf;
  int adx;

  if (ys > ye) {
    swapr(xs, xe);
    swapr(ys, ye);
  }

  dx = xe - xs;
  dy = ye - ys;

  if (dy == 0)
    return;

  pixels += ys * stride;
  pixels += xs >> 3;

  adx = absw(dx);

  if (adx < dy) {
    di = 0;
    df = adx;
  } else {
    divmod16(adx, dy, df, di);
  }

  xi = ~xs & 7;
  xf = -dy;
  n = dy - 1;

  if (dx >= 0) {
    di = di & 7;
    stride += di >> 3;

    do {
      bchg(pixels, xi);

      pixels += stride;

      xf += df;
      if (xf >= 0) {
        xi--;
        xf -= dy;
      }

      xi -= di;
      if (xi < 0) {
        pixels++;
        xi += 8;
      }
    } while (--n != -1);
  } else {
    stride -= di >> 3;
    di = -(di & 7);
    xi -= 8;

    do {
      bchg(pixels, xi);

      pixels += stride;

      xf += df;
      if (xf >= 0) {
        xi++;
        xf -= dy;
      }

      xi -= di;
      if (xi >= 0) {
        pixels--;
        xi -= 8;
      }
    } while (--n != -1);
  }
}
