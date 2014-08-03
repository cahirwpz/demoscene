#include "blitter.h"
#include "hardware.h"

__regargs void BlitterClear(BitmapT *bitmap, UWORD plane) {
  custom->bltdpt = bitmap->planes[plane];
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltsize = (bitmap->height << 6) + (bitmap->width >> 4);
}

__regargs void BlitterFill(BitmapT *bitmap, UWORD plane) {
  UBYTE *bpl = bitmap->planes[plane] + bitmap->bplSize - 1;

  custom->bltapt = bpl;
  custom->bltdpt = bpl;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = (bitmap->height << 6) + (bitmap->width >> 4);
}

/*
 * Minterm is either:
 * - OR: (ABC | ABNC | NABC | NANBC)
 * - XOR: (ABNC | NABC | NANBC)
 */

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

__regargs void BlitterLine(BitmapT *bitmap, UWORD plane,
                           UWORD bltcon0, UWORD bltcon1, Line2D *line)
{
  UBYTE *data = bitmap->planes[plane];
  WORD bwidth = bitmap->width / 8;
  WORD dx, dy, dmax, dmin, derr;
  WORD x1 = line->x1;
  WORD x2 = line->x2;
  WORD y1 = line->y1;
  WORD y2 = line->y2;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  dx = abs(x2 - x1);
  dy = y2 - y1;

  if (dx >= dy) {
    dmax = dx;
    dmin = dy;
    if (x1 >= x2)
      bltcon1 |= AUL;
    bltcon1 |= SUD;
  } else {
    dmax = dy;
    dmin = dx;
    if (x1 >= x2)
      bltcon1 |= SUL;
  }

  derr = 4 * dmin - 2 * dmax;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;

  custom->bltamod = 4 * dmin - 4 * dmax;
  custom->bltbmod = 4 * dmin;
  custom->bltcmod = bwidth;
  custom->bltdmod = bwidth;

  /* Word containing the first pixel of the line. */
  data += bwidth * y1;
  data += (x1 >> 3) & ~1;

  custom->bltapt = (APTR)(LONG)derr;
  custom->bltcpt = data;
  /* Uses undocumented chipset feature.
   * First dot is drawn into bltdpt, the rest goes to bltcpt. */
  custom->bltdpt = (bltcon1 & ONEDOT) ? bitmap->planes[bitmap->depth] : data;

  custom->bltcon0 = rorw(x1 & 15, 4) | bltcon0;
  custom->bltcon1 = rorw(x1 & 15, 4)  | bltcon1;

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */

  custom->bltsize = dmax * 64 + 66;
}
