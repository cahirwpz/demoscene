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
  u_char *data;
  u_char *scratch;
  short stride;
  u_short bltcon0;
  u_short bltcon1;
} line[1];

void BlitterLineSetupFull(const BitmapT *bitmap, u_short plane,
                          u_short mode, u_short pattern)
{
  line->data = bitmap->planes[plane];
  line->scratch = bitmap->planes[bitmap->depth];
  line->stride = bitmap->bytesPerRow;
  line->bltcon0 = LineMode[mode][0];
  line->bltcon1 = LineMode[mode][1];

  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = pattern; /* Line texture pattern. */
  custom->bltcmod = line->stride;
  custom->bltdmod = line->stride;
}

void BlitterLine(short x1 asm("d2"), short y1 asm("d3"), short x2 asm("d4"), short y2 asm("d5")) {
  u_char *data = line->data;
  u_short bltcon1 = line->bltcon1;
  short dx, dy, derr;

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
    u_short bltcon0 = rorw(x1 & 15, 4) | line->bltcon0;
    u_short bltamod = derr - dx;
    u_short bltbmod = dy + dy;
    void *bltdpt = (bltcon1 & ONEDOT) ? line->scratch : data;
    u_short bltsize = (dx << 6) + 66;

    WaitBlitter();

    custom->bltcon0 = bltcon0;
    custom->bltcon1 = bltcon1;
    custom->bltamod = bltamod;
    custom->bltbmod = bltbmod;
    custom->bltapt = (void *)(int)derr;
    custom->bltcpt = data;
    custom->bltdpt = bltdpt;
    custom->bltsize = bltsize;
  }
}
