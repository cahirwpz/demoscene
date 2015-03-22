#include "blitter.h"
#include "hardware.h"

__regargs void BlitterFillArea(BitmapT *bitmap, WORD plane, Area2D *area) {
  APTR bltpt = bitmap->planes[plane];
  UWORD bltmod, bltsize;

  if (area) {
    WORD x = area->x;
    WORD y = area->y; 
    WORD w = area->w >> 3;
    WORD h = area->h;

    bltpt += (((x + w) >> 3) & ~1) + ((y + h) * bitmap->bytesPerRow);
    bltmod = bitmap->bytesPerRow - w;
    bltsize = (h << 6) | (w >> 1);
  } else {
    bltpt += bitmap->bplSize;
    bltmod = 0;
    bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
  }

  bltpt -= 2;

  WaitBlitter();

  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltamod = bltmod;
  custom->bltdmod = bltmod;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = bltsize;
}

__regargs void BlitterSetArea(BitmapT *bitmap, WORD plane,
                              Area2D *area, UWORD pattern)
{
  APTR bltpt = bitmap->planes[plane];
  UWORD bltmod, bltsize, bltshift;

  if (area) {
    WORD x = area->x;
    WORD y = area->y; 
    WORD w = area->w >> 3;
    WORD h = area->h;

    bltpt += ((x >> 3) & ~1) + y * bitmap->bytesPerRow;

    bltshift = rorw(x & 15, 4);
    bltmod = bitmap->bytesPerRow - w;
    bltsize = (h << 6) | (w >> 1);
  } else {
    bltshift = 0;
    bltmod = 0;
    bltsize = (bitmap->height << 6) | (bitmap->bytesPerRow >> 1);
  }

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; bltmod -= 2;

    custom->bltadat = 0xffff;
    custom->bltbpt = bltpt;
    custom->bltcdat = pattern;
    custom->bltbmod = -2;
    custom->bltcmod = bltmod;
    custom->bltcon0 = (SRCB | DEST) | (NABC | NABNC | ABC | ANBC) | bltshift;
    custom->bltcon1 = bltshift;

    custom->bltdpt = bltpt;
    custom->bltdmod = bltmod;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  } else {
    custom->bltadat = pattern;
    custom->bltcon0 = DEST | A_TO_D;
    custom->bltcon1 = 0;

    custom->bltdpt = bltpt;
    custom->bltdmod = bltmod;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  }
}

void BlitterSetMaskArea(BitmapT *dst, WORD dstbpl, UWORD x, UWORD y,
                        BitmapT *msk, Area2D *area, UWORD pattern)
{
  APTR dstbpt = dst->planes[dstbpl];
  APTR mskbpt = msk->planes[0];
  UWORD dstmod, mskmod, bltsize, bltshift;

  if (area) {
    /* TODO: handle unaligned mx */
    WORD mx = area->x;
    WORD my = area->y;
    WORD mw = area->w;
    WORD mh = area->h;
    WORD bytesPerRow = ((mw + 15) & ~15) >> 3;

    dstbpt += ((x >> 3) & ~1) + y * dst->bytesPerRow;
    mskbpt += ((mx >> 3) & ~1) + my * msk->bytesPerRow;
    dstmod = dst->bytesPerRow - bytesPerRow;
    mskmod = msk->bytesPerRow - bytesPerRow;
    bltsize = (mh << 6) | (bytesPerRow >> 1);
    bltshift = rorw(x & 15, 4);
  } else {
    dstbpt += ((x >> 3) & ~1) + y * dst->bytesPerRow;
    dstmod = dst->bytesPerRow - msk->bytesPerRow;
    mskmod = dstmod;
    bltsize = (msk->height << 6) | (msk->bytesPerRow >> 1);
    bltshift = rorw(x & 15, 4);
  }

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; dstmod -= 2; mskmod -= 2;

    custom->bltbmod = mskmod;
    custom->bltcon0 = (SRCB|SRCC|DEST) | (ABC|ABNC|ANBC|NANBC) | bltshift;
    custom->bltcon1 = bltshift;
    custom->bltalwm = 0;

    custom->bltadat = pattern;
    custom->bltbpt = mskbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;
    custom->bltafwm = -1;
    custom->bltsize = bltsize;
  } else {
    custom->bltbmod = 0;
    custom->bltcon0 = (SRCB|SRCC|DEST) | (ABC|ABNC|ANBC|NANBC);
    custom->bltcon1 = 0;
    custom->bltalwm = -1;

    custom->bltadat = pattern;
    custom->bltbpt = mskbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;
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

void BlitterLine(WORD x1 asm("d2"), WORD y1 asm("d3"), WORD x2 asm("d4"), WORD y2 asm("d5")) {
  UBYTE *data = line.data;
  UWORD bltcon1 = line.bltcon1;
  WORD dx, dy, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  data += line.stride * y1;
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
    UWORD bltcon0 = rorw(x1 & 15, 4) | line.bltcon0;
    UWORD bltamod = derr - dx;
    UWORD bltbmod = dy + dy;
    APTR bltdpt = (bltcon1 & ONEDOT) ? line.scratch : data;
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
