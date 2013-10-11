#include "blitter.h"
#include "hardware.h"

__regargs void BlitterClear(BitmapT *bitmap, UWORD b) {
  custom->bltdpt = bitmap->planes[b];
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltsize = (bitmap->height * 64) + (bitmap->width / 16);
}

__regargs void BlitterLine(BitmapT *bitmap, UWORD b,
                           UWORD x1, UWORD y1, UWORD x2, UWORD y2)
{
  UBYTE *data = bitmap->planes[b];
  UWORD bwidth = bitmap->width / 8;
  WORD dx, dy, dmax, dmin, bltapt, begin;

  dx = x2 - x1;
  if (dx < 0)
    dx = -dx;

  dy = y2 - y1;
  if (dy < 0)
    dy = -dy;

  if (dx > dy) {
    dmax = dx;
    dmin = dy;
  } else {
    dmax = dy;
    dmin = dx;
  }

  bltapt = (dmin << 2) - (dmax << 1);
  custom->bltapt = (APTR)(LONG)bltapt;

  custom->bltamod = (dmin - dmax) << 2;
  custom->bltbmod = dmin << 2;
  custom->bltcmod = bwidth;
  custom->bltdmod = bwidth;

  /* Word containing the first pixel of the line. */
  begin = (UWORD)(bwidth * y1) + ((x1 >> 3) ^ 1);
  data = &data[begin];

  custom->bltcpt = data;
  custom->bltdpt = data;

  /* Minterm is either $ca or $4a */
  custom->bltcon0 = ((x1 & 15) << 12) |
    (SRCA | SRCC | DEST) | (ABC | ABNC | NABC | NANBC); /* $ca */

  /*
   *  \   |   /
   *   \3 | 1/
   *  7 \ | / 6
   *     \|/
   *  ----X----
   *     /|\
   *  5 / | \ 4
   *   /2 | 0\
   *  /   |   \
   *
   * OCT | SUD SUL AUL
   * ----+------------
   *   3 |   1   1   1
   *   0 |   1   1   0
   *   4 |   1   0   1
   *   7 |   1   0   0
   *   2 |   0   1   1
   *   5 |   0   1   0
   *   1 |   0   0   1
   *   6 |   0   0   0
   */

  {
    UWORD bltcon1 = ((x1 & 15) << 12) | LINEMODE;

    if (bltapt < 0)
      bltcon1 |= SIGNFLAG;

    if (dx >= dy) {
      if (x1 >= x2)
        bltcon1 |= AUL;
      if (y1 >= y2)
        bltcon1 |= SUL;
      bltcon1 |= SUD;
    } else {
      if (x1 >= x2)
        bltcon1 |= SUL;
      if (y1 >= y2)
        bltcon1 |= AUL;
    }

    custom->bltcon1 = bltcon1;
  }

  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltsize = ((dmax + 1) << 6) + 2;
}
