#include "blitter.h"
#include "hardware.h"

__regargs void BlitterClear(BitmapT *bitmap, WORD plane) {
  register APTR bltpt asm("a1") = bitmap->planes[plane];
  UWORD bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);

  WaitBlitter();

  custom->bltadat = 0;
  custom->bltdpt = bltpt;
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltsize = bltsize;
}


__regargs void BlitterFill(BitmapT *bitmap, WORD plane) {
  register APTR bltpt asm("a1") = bitmap->planes[plane] + bitmap->bplSize - 2;
  UWORD bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);

  WaitBlitter();

  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = bltsize;
}

void BlitterSet(BitmapT *dst, WORD dstbpl, UWORD x, UWORD y, UWORD w, UWORD h, UWORD val) {
  APTR dstbpt = dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD bltsize = (h << 6) | (w >> 4);
  UWORD bltmod = dst->bytesPerRow - (w >> 3);

  x = rorw(x & 15, 4);

  WaitBlitter();

  if (x) {
    bltsize += 1; bltmod -= 2;

    custom->bltadat = 0xffff;
    custom->bltbpt = dstbpt;
    custom->bltcdat = val;
    custom->bltbmod = -2;
    custom->bltcmod = bltmod;
    custom->bltcon0 = (SRCB | DEST) | (NABC | NABNC | ABC | ANBC) | x;
    custom->bltcon1 = x;

    custom->bltdpt = dstbpt;
    custom->bltdmod = bltmod;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  } else {
    custom->bltadat = val;
    custom->bltamod = 0;
    custom->bltcon0 = DEST | A_TO_D;
    custom->bltcon1 = 0;

    custom->bltdpt = dstbpt;
    custom->bltdmod = bltmod;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  }
}

void BlitterSetMask(BitmapT *dst, WORD dstbpl, UWORD x, UWORD y,
                    BitmapT *msk, UWORD val)
{
  APTR dstbpt = dst->planes[dstbpl] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  APTR mskbpt = msk->planes[0];
  UWORD bltmod = dst->bytesPerRow - msk->bytesPerRow;
  UWORD bltsize = (msk->height << 6) | (msk->bytesPerRow >> 1);

  x = rorw(x & 15, 4);

  WaitBlitter();

  if (x) {
    bltsize += 1; bltmod -= 2;

    custom->bltbmod = -2;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC) | x;
    custom->bltcon1 = x;
    custom->bltalwm = 0;

    custom->bltadat = val;
    custom->bltbpt = mskbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltcmod = bltmod;
    custom->bltdmod = bltmod;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  } else {
    custom->bltbmod = 0;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC);
    custom->bltcon1 = 0;
    custom->bltalwm = -1;

    custom->bltadat = val;
    custom->bltbpt = mskbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltcmod = bltmod;
    custom->bltdmod = bltmod;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  }
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

struct {
  UBYTE *data;
  UBYTE *scratch;
  WORD stride;
  UWORD bltcon0;
  UWORD bltcon1;
} line;

__regargs void BlitterLineSetup(BitmapT *bitmap, UWORD plane, UWORD bltcon0, UWORD bltcon1) 
{
  line.data = bitmap->planes[plane];
  line.scratch = bitmap->planes[bitmap->depth];
  line.stride = bitmap->bytesPerRow;
  line.bltcon0 = bltcon0;
  line.bltcon1 = bltcon1;

  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = line.stride;
  custom->bltdmod = line.stride;
}

#if 0
__regargs void BlitterLine(WORD x1, WORD y1, WORD x2, WORD y2) {
  UBYTE *data = line.data;
  UWORD bltcon1 = line.bltcon1;
  WORD dmax, dmin, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  data += line.stride * y1;
  data += (x1 >> 3) & ~1;

  dmax = x2 - x1;
  dmin = y2 - y1;

  if (dmax < 0)
    dmax = -dmax;

  if (dmax >= dmin) {
    if (x1 >= x2)
      bltcon1 |= (AUL | SUD);
    else
      bltcon1 |= SUD;
  } else {
    swapr(dmax, dmin);
    if (x1 >= x2)
      bltcon1 |= SUL;
  }

  derr = 2 * dmin - dmax;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;

  custom->bltcon0 = rorw(x1 & 15, 4) | line.bltcon0;
  custom->bltcon1 = rorw(x1 & 15, 4) | bltcon1;

  custom->bltamod = derr - dmax;
  custom->bltbmod = 2 * dmin;

  custom->bltapt = (APTR)(LONG)derr;
  custom->bltcpt = data;
  /* Uses undocumented chipset feature.
   * First dot is drawn into bltdpt, the rest goes to bltcpt. */
  custom->bltdpt = (bltcon1 & ONEDOT) ? line.scratch : data;

  custom->bltsize = (dmax << 6) + 66;
}
#endif

__regargs void BlitterLine(WORD x1, WORD y1, WORD x2, WORD y2) {
  UBYTE *data = line.data;
  UWORD bltcon1 = line.bltcon1;
  WORD dmax, dmin, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  data += line.stride * y1;
  data += (x1 >> 3) & ~1;

  dmax = x2 - x1;
  dmin = y2 - y1;

  if (dmax < 0)
    dmax = -dmax;

  if (dmax >= dmin) {
    if (x1 >= x2)
      bltcon1 |= (AUL | SUD);
    else
      bltcon1 |= SUD;
  } else {
    swapr(dmax, dmin);
    if (x1 >= x2)
      bltcon1 |= SUL;
  }

  derr = 2 * dmin - dmax;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;
  bltcon1 |= rorw(x1 & 15, 4);

  {
    UWORD bltcon0 = rorw(x1 & 15, 4) | line.bltcon0;
    UWORD bltamod = derr - dmax;
    UWORD bltbmod = 2 * dmin;
    APTR bltapt = (APTR)(LONG)derr;
    APTR bltdpt = (bltcon1 & ONEDOT) ? line.scratch : data;
    UWORD bltsize = (dmax << 6) + 66;

    WaitBlitter();

    custom->bltcon0 = bltcon0;
    custom->bltcon1 = bltcon1;
    custom->bltamod = bltamod;
    custom->bltbmod = bltbmod;
    custom->bltapt = bltapt;
    custom->bltcpt = data;
    custom->bltdpt = bltdpt;
    custom->bltsize = bltsize;
  }
}
