#include "gfx/line.h"

void draw_line(canvas_t *canvas, int xs, int ys, int xe, int ye)
{
  int stride = canvas->bitmap->width;
  uint8_t *bitmap = &canvas->bitmap->data[ys * stride + xs];

  if (ys > ye) {
    swapi(xs, xe);
    swapi(ys, ye);
  }

  int dx = abs(xe - xs);
  int dy = ye - ys;

  /*  quarters:
   *
   *  \ 1 | 3 /
   *   \  |  /
   *    \ | /
   *  0  \|/  2
   * -----x-----
   *  2  /|\  0
   *    / | \
   *   /  |  \
   *  / 3 | 1 \
   *
   */

  int q = ((xe < xs) << 1) | (dx < dy);

  int dg, dg1, dg2, db1, db2, steps;

  if (q & 1) {
    dg1 = (dx - dy) << 1;
    dg2 = (dx << 1);
    dg  = dg2 - dy;

    db1 = (q & 2) ? -1 : 1;
    db2 = stride;

    steps = dy;
  } else {
    dg1 = (dy - dx) << 1;
    dg2 = (dy << 1);
    dg  = dg2 - dx;

    db1 = stride;
    db2 = (q & 2) ? -1 : 1;

    steps = dx;
  }

  uint8_t color = canvas->fg_col;

  for (;;) {
    *bitmap = color;

    if (!steps--)
      break;

    if (dg > 0) {
      bitmap += db1;
      dg += dg1;
    } else {
      dg += dg2;
    }

    bitmap += db2;
  }
}
