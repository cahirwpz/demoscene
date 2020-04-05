#include "blitter.h"

void BlitterSetMaskArea(const BitmapT *dst, short dstbpl, u_short x, u_short y,
                        const BitmapT *msk, const Area2D *area, u_short pattern)
{
  void *dstbpt = dst->planes[dstbpl];
  void *mskbpt = msk->planes[0];
  u_short dstmod, mskmod, bltsize, bltshift;

  if (area) {
    /* TODO: handle unaligned mx */
    short mx = area->x;
    short my = area->y;
    short mw = area->w;
    short mh = area->h;
    short bytesPerRow = ((mw + 15) & ~15) >> 3;

    dstbpt += ((x >> 3) & ~1) + (short)y * (short)dst->bytesPerRow;
    mskbpt += ((mx >> 3) & ~1) + (short)my * (short)msk->bytesPerRow;
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
