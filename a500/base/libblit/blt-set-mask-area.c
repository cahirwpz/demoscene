#include "blitter.h"

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

    dstbpt += ((x >> 3) & ~1) + (WORD)y * (WORD)dst->bytesPerRow;
    mskbpt += ((mx >> 3) & ~1) + (WORD)my * (WORD)msk->bytesPerRow;
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
