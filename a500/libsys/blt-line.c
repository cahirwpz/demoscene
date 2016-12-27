#include "blitter.h"
#include "hardware.h"

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

struct {
  UBYTE *data;
  UBYTE *scratch;
  WORD stride;
  UWORD bltcon0;
  UWORD bltcon1;
} line[1];

void BlitterLineSetup(BitmapT *bitmap, UWORD plane, UWORD bltcon0, UWORD bltcon1) 
{
  line->data = bitmap->planes[plane];
  line->scratch = bitmap->planes[bitmap->depth];
  line->stride = bitmap->bytesPerRow;
  line->bltcon0 = bltcon0;
  line->bltcon1 = bltcon1;

  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = line->stride;
  custom->bltdmod = line->stride;
}

void BlitterLine(WORD x1 asm("d2"), WORD y1 asm("d3"), WORD x2 asm("d4"), WORD y2 asm("d5")) {
  UBYTE *data = line->data;
  UWORD bltcon1 = line->bltcon1;
  WORD dx, dy, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  data += line->stride * y1;
  data += (x1 >> 3) & ~1;

  dx = x2 - x1;
  dy = y2 - y1;

  if (dx < 0) {
    dx = -dx;
    if (dx >= dy) {
      bltcon1 |= AUL | SUD;
    } else {
      bltcon1 |= SUL;
      swapr(dx, dy);
    }
  } else {
    if (dx >= dy) {
      bltcon1 |= SUD;
    } else {
      swapr(dx, dy);
    }
  }

  derr = dy + dy - dx;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;

  {
    UWORD bltcon0 = rorw(x1 & 15, 4) | line->bltcon0;
    UWORD bltamod = derr - dx;
    UWORD bltbmod = dy + dy;
    APTR bltdpt = (bltcon1 & ONEDOT) ? line->scratch : data;
    UWORD bltsize = (dx << 6) + 66;

    WaitBlitter();

    custom->bltcon0 = bltcon0;
    custom->bltcon1 = bltcon1;
    custom->bltamod = bltamod;
    custom->bltbmod = bltbmod;
    custom->bltapt = (APTR)(LONG)derr;
    custom->bltcpt = data;
    custom->bltdpt = bltdpt;
    custom->bltsize = bltsize;
  }
}
