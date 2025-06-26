/*
 * Operations on bitplanes that Blitter can perform
 */
static class Blit {
  static void zeros(Bitplane dst) {
    for (int i = 0; i < dst.size(); i++)
      dst.write(i, 0);
  }

  static void ones(Bitplane dst) {
    for (int i = 0; i < dst.size(); i++)
      dst.write(i, ~0);
  }
  
  static void set(Bitplane dst, int x, int y, int w, int h, boolean value) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        dst.set(x + i, y + j, value);
      }
    }
  }
  
  static void copy(Bitplane dst, Bitplane src, int x, int y) {
    for (int j = 0; j < src.height; j++) {
      for (int i = 0; i < src.width; i++) {
        dst.set(x + i, y + j, src.get(i, j));
      }
    }
  }

  static void copy(Bitplane dst, Bitplane src, int sx, int sy, int sw, int sh, int dx, int dy) {
    for (int j = 0; j < sh; j++) {
      for (int i = 0; i < sw; i++) {
        dst.set(i + dx, j + dy, src.get(i + sx, j + sy));
      }
    }
  }

  static void and(Bitplane dst, Bitplane src) {
    for (int i = 0; i < dst.size(); i++)
      dst.write(i, dst.read(i) & src.read(i));
  }

  static void and(Bitplane dst, Bitplane src, int x, int y) {
    for (int j = 0; j < src.height; j++)
      for (int i = 0; i < src.width; i++)
        dst.set(x + i, y + j, dst.get(x + i, y + j) & src.get(i, j)); 
  }

  static void or(Bitplane dst, Bitplane src) {
    for (int i = 0; i < dst.size(); i++)
      dst.write(i, dst.read(i) | src.read(i));
  }
  
  static void or(Bitplane dst, Bitplane src, int x, int y) {
    for (int j = 0; j < src.height; j++)
      for (int i = 0; i < src.width; i++)
        dst.set(x + i, y + j, dst.get(x + i, y + j) | src.get(i, j));
  }

  static void or_mask(Bitplane dst, Bitplane src, Bitplane mask, int x, int y) {
    for (int j = 0; j < src.height; j++)
      for (int i = 0; i < src.width; i++) {
        boolean a = dst.get(x + i, y + j);
        boolean b = src.get(i, j);
        boolean c = mask.get(i, j);
        boolean d = (a & !c) | (b & c);
        dst.set(x + i, y + j, d);
      }
  }

  static void not(Bitplane dst) {
    for (int i = 0; i < dst.size(); i++)
      dst.write(i, ~dst.read(i));
  }

  static void add(Bitplane dst, Bitplane src, int dx, int dy, Bitplane carry) {
    for (int j = 0; j < src.height; j++) {
      for (int i = 0; i < src.width; i++) {
        boolean a = dst.get(dx + i, dy + j);
        boolean b = src.get(i, j);
        boolean d1 = (a & !b) | (!a & b);
        boolean d2 = a & b;
        dst.set(dx + i, dy + j, d1);
        carry.set(i, j, d2);
      }
    }
  }

  static void addx(Bitplane dst, Bitplane src, Bitplane carry_in, int dx, int dy, Bitplane carry_out) {
    for (int j = 0; j < src.height; j++) {
      for (int i = 0; i < src.width; i++) {
        boolean a = dst.get(dx + i, dy + j);
        boolean b = src.get(i, j);
        boolean c = carry_in.get(i, j);
        boolean d1 = (!a & !b & c) | (!a & b & !c) | (a & !b & !c) | (a & b & c);
        boolean d2 = (!a & b & c) | (a & !b & c) | (a & b & !c) | (a & b & c);
        dst.set(dx + i, dy + j, d1);
        carry_out.set(i, j, d2);
      }
    }
  }

  static void sub(Bitplane dst, Bitplane src, Bitplane borrow) {
    for (int i = 0; i < dst.size(); i++) {
      int a = dst.read(i);
      int b = src.read(i);
      dst.write(i, (a & ~b) | (~a & b));
      borrow.write(i, ~a & b);
    }
  }
  
  static void sub(Bitplane dst, Bitplane src, int x, int y, Bitplane borrow) {
    for (int j = 0; j < src.height; j++) {
      for (int i = 0; i < src.width; i++) {
        boolean a = dst.get(x + i, y + j);
        boolean b = src.get(i, j);
        boolean d1 = (a & !b) | (!a & b);
        boolean d2 = !a & b;
        dst.set(x + i, y + j, d1);
        borrow.set(i, j, d2);
      }
    }
  }

  static void subx(Bitplane dst, Bitplane src, Bitplane borrow_in, int x, int y, Bitplane borrow_out) {
    for (int j = 0; j < src.height; j++) {
      for (int i = 0; i < src.width; i++) {
        boolean a = dst.get(x + i, y + j);
        boolean b = src.get(i, j);
        boolean c = borrow_in.get(i, j);
        boolean d1 = (!a & !b & c) | (!a & b & !c) | (a & !b & !c) | (a & b & c);
        boolean d2 = (!a & !b & c) | (!a & b & !c) | (a & b & c);
        dst.set(x + i, y + j, d1);
        borrow_out.set(i, j, d2);
      }
    }
  }

  static void rshift(Bitplane dst, int n) {
    assert n < 16;
    
    for (int j = 0; j < dst.height; j++) {
      int i = dst.width - 1;
      for (; i >= n; i--)
        dst.set(i, j, dst.get(i - n, j));
      for (; i >= 0; i--)
        dst.bclr(i, j);
    }
  }

  static void lshift(Bitplane dst, int n) {
    assert n < 16;
    
    for (int j = 0; j < dst.height; j++) {
      int i = 0;
      for (; i < dst.width - n; i++)
        dst.set(i, j, dst.get(i + n, j));
      for (; i < dst.width; i++)
        dst.bclr(i, j);
    }
  }
  
  static void fill(Bitplane dst) {
    for (int y = 0; y < dst.height; y++) {
      boolean p = false;
      for (int x = dst.width - 1; x >= 0; x--) {
        boolean q = dst.get(x, y);
        p ^= q;
        dst.set(x, y, p);
      }
    }
  }
  
  static void line(Bitplane dst, int xs, int ys, int xe, int ye) {
    if (ys > ye) {
      int xt = xs; xs = xe; xe = xt;
      int yt = ys; ys = ye; ye = yt;
    }

    int s = (xs < xe) ? 1 : -1;
    int dx = abs(xe - xs);
    int dy = ye - ys;
    int dg1 = 2 * dx;
    int dg2 = 2 * dy;

    if (dx < dy) {
      int dg = 2 * dx - dy;

      do {
        dst.bset(xs, ys);

        if (dg > 0) {
          xs += s;
          dg += dg1 - dg2;
        } else {
          dg += dg1;
        }
        ys++;
      } while (--dy > 0);
    } else {
      int dg = 2 * dy - dx;

      do {
        dst.bset(xs, ys);

        if (dg > 0) {
          ys++;
          dg += dg2 - dg1;
        } else {
          dg += dg2;
        }
        xs += s;
      } while (--dx > 0);
    }
  }

  static void lineE(Bitplane dst, int x1, int y1, int x2, int y2) {
    if (y1 > y2) {
      int xt = x1; x1 = x2; x2 = xt;
      int yt = y1; y1 = y2; y2 = yt;
    }

    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dy == 0)
      return;

    int di = dx / dy;
    int df = abs(dx) % dy;
    int xi = x1;
    int xf = 0;
    int s = (dx >= 0) ? 1 : -1;

    while (y1 < y2) {
      dst.bxor(xi, y1++);
      xi += di;
      xf += df;
      if (xf > dy) {
        xf -= dy;
        xi += s;
      }
    }
  }
};

/*
 * CPU to bitplane drawing routines
 */
 
static class Draw {
  static void circle(Bitplane dst, int x0, int y0, int r) {
    int x = -r;
    int y = 0;
    int err = 2 * (1 - r);

    do {
      dst.bxor(x0 - x, y0 + y);
      dst.bxor(x0 - y, y0 - x);
      dst.bxor(x0 + x, y0 - y);
      dst.bxor(x0 + y, y0 + x);

      if (err <= y) {
        y++;
        err += y * 2 + 1;
      }
      if (err > x) {
        x++;
        err += x * 2 + 1;
      }
    } while (x < 0);
  }

  static void circleE(Bitplane dst, int x0, int y0, int r) {
    int x = -r;
    int y = 0;
    int err = 2 * (1 - r);

    do {
      if (err <= y) {
        dst.bxor(x0 - x, y0 - y);
        dst.bxor(x0 + x, y0 - y);
        if (y != 0) {
          dst.bxor(x0 + x, y0 + y);
          dst.bxor(x0 - x, y0 + y);
        }
        y++;
        err += 2 * y + 1;
      }
      if (err > x) {
        x++;
        err += 2 * x + 1;
      }
    } while (x < 0);
  }

  /* Generating Conic Sections Using an Efficient Algorithms
   * by Abdul-Aziz Solyman Khalil */
  static void ellipse(Bitplane dst, int xc, int yc, int rx, int ry) {
    int rx2, ry2, tworx2, twory2, x_slop, y_slop;
    int d, mida, midb;
    int x, y;

    x = 0; 
    y = ry;
    rx2 = rx * rx; 
    ry2 = ry * ry;
    tworx2 = 2 * rx2; 
    twory2 = 2 * ry2;
    x_slop = 2 * twory2; 
    y_slop = 2 * tworx2 * (y - 1);
    mida = rx2 / 2;
    midb = ry2 / 2;
    d = twory2 - rx2 - y_slop / 2 - mida;

    while (d <= y_slop) {
      dst.bset(xc + x, yc + y);
      dst.bset(xc - x, yc + y);
      dst.bset(xc + x, yc - y);
      dst.bset(xc - x, yc - y);
      if (d > 0) {
        d -= y_slop;
        y--;
        y_slop -= 2 * tworx2;
      }
      d += twory2 + x_slop;
      x++;
      x_slop += 2 * twory2;
    }

    d -= (x_slop + y_slop) / 2 + (ry2 - rx2) + (mida - midb);

    while (y >= 0) {
      dst.bset(xc + x, yc + y);
      dst.bset(xc - x, yc + y);
      dst.bset(xc + x, yc - y);
      dst.bset(xc - x, yc - y);

      if (d <= 0) {
        d += x_slop;
        x++;
        x_slop += 2 * twory2;
      }
      d += tworx2 - y_slop;
      y--;
      y_slop -= 2 * tworx2;
    }
  }

  static void hyperbola(Bitplane dst, int xc, int yc, int rx, int ry, int bound) {
    int x, y, d, mida, midb;
    int tworx2, twory2, rx2, ry2;
    int x_slop, y_slop;

    x = rx; 
    y = 0;
    rx2 = rx * rx; 
    ry2 = ry * ry;
    tworx2 = 2 * rx2; 
    twory2 = 2 * ry2;
    x_slop = 2 * twory2 * ( x + 1 );
    y_slop = 2 * tworx2;
    mida = x_slop / 2; 
    midb = y_slop / 2;
    d = tworx2 - ry2 * (1 + 2 * rx) + midb;

    while ((d < x_slop) && (y <= bound)) {
      dst.bset(xc + x, yc + y);
      dst.bset(xc + x, yc - y);
      dst.bset(xc - x, yc + y);
      dst.bset(xc - x, yc - y);
      if (d >= 0) {
        d -= x_slop;
        x++;
        x_slop += 2 * tworx2;
      }
      d += tworx2 + y_slop;
      y++;
      y_slop += 2 * tworx2;
    }

    d -= (x_slop + y_slop) / 2 + (rx2 + ry2) - (mida + midb);

    if (rx > ry) {
      while (y <= bound) {
        dst.bset(xc + x, yc + y);
        if (d <= 0) {
          d += y_slop;
          y++;
          y += 2 * tworx2;
        }
        d -= twory2 - x_slop;
        x++;
        x_slop += 2 * twory2;
      }
    }
  }

  static void parabola(Bitplane dst, int x0, int y0, int p, int bound) {
    int p2 = 2 * p;
    int p4 = 2 * p2;
    int x = 0;
    int y = 0;
    int d = 1 - p;

    while ((y < p) && (x <= bound)) {
      dst.bset(x0 + x, y0 - y);
      dst.bset(x0 + x, y0 + y);
      if (d >= 0) {
        x++;
        d -= p2;
      }
      y++;
      d += 2 * y + 1;
    }

    d = 1 - (d == 1 ? p4 : p2);

    while (x <= bound) {
      dst.bset(x0 + x, y0 - y);
      dst.bset(x0 + x, y0 + y);
      if (d <= 0) {
        y++;
        d += 4 * y;
      }
      x++;
      d -= p4;
    }
  }
};
