#include "blitter.h"
#include "hardware.h"

__regargs void BlitterLine(BitmapT *bitmap, UWORD b,
                           UWORD x1, UWORD y1, UWORD x2, UWORD y2)
{
  UWORD dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
  UWORD dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
  UWORD dmax = max(dx, dy);
  UWORD dmin = min(dx, dy);
  UWORD bwidth = bitmap->width / 8;
  LONG bltapt = 4 * dmin - 2 * dmax;

  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltafwm = 0xffff;
  custom->bltalwm = 0xffff;

  custom->bltamod = 4 * (dmin - dmax);
  custom->bltbmod = 4 * dmin;
  custom->bltcmod = bwidth;
  custom->bltdmod = bwidth;

  custom->bltapt = (APTR)bltapt;

  /* Word containing the first pixel of the line. */
  {
    UBYTE *data = bitmap->planes[b];

    data += bwidth * y1 + ((x1 / 8) ^ 1);

    custom->bltcpt = data;
    custom->bltdpt = data;
  }

  /* Minterm is either $ca or $4a */
  custom->bltcon0 =
    ((x1 & 15) << 12) | BLTCON0_USEA | BLTCON0_USEC | BLTCON0_USED | 0xca;

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
    UWORD bltcon1 = ((x1 & 15) << 12) | BLTCON1_LINE;

    if (bltapt < 0)
      bltcon1 |= BLTCON1_SIGN;

    if (dx >= dy) {
      if (x1 >= x2)
        bltcon1 |= BLTCON1_AUL;
      if (y1 >= y2)
        bltcon1 |= BLTCON1_SUL;
      bltcon1 |= BLTCON1_SUD;
    } else {
      if (x1 >= x2)
        bltcon1 |= BLTCON1_SUL;
      if (y1 >= y2)
        bltcon1 |= BLTCON1_AUL;
    }

    custom->bltcon1 = bltcon1;
  }

  custom->bltsize = (dmax + 1) * 64 + 2;
}
