#include "gfx.h"
#include "blitter.h"

__regargs void BitmapCopy(BitmapT *dst, UWORD x, UWORD y, BitmapT *src)
{
  APTR srcbpt = src->planes[0];
  APTR dstbpt = dst->planes[0] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD dstmod = dst->bytesPerRow - src->bytesPerRow;
  UWORD bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  UWORD bltshift = rorw(x & 15, 4);
  WORD n = src->depth;

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; dstmod -= 2; 
  
    custom->bltadat = 0xffff;
    custom->bltbmod = -2;
    custom->bltcmod = dstmod;
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC) | bltshift;
    custom->bltcon1 = bltshift;
    custom->bltalwm = 0;
    custom->bltafwm = -1;
    custom->bltdmod = dstmod;
  } else {
    custom->bltamod = 0;
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltdmod = dstmod;
  }

  for (; --n >= 0; srcbpt += src->bplSize, dstbpt += dst->bplSize) {
    WaitBlitter();

    if (bltshift) {
      custom->bltbpt = srcbpt;
      custom->bltcpt = dstbpt;
      custom->bltdpt = dstbpt;
      custom->bltsize = bltsize;
    } else {
      custom->bltapt = srcbpt;
      custom->bltdpt = dstbpt;
      custom->bltsize = bltsize;
    }
  }
}

__regargs void BitmapCopyFast(BitmapT *dst, UWORD x, UWORD y, BitmapT *src)
{
  APTR srcbpt = src->planes[0];
  APTR dstbpt = dst->planes[0] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD dstmod = dst->bytesPerRow - src->bytesPerRow;
  UWORD bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  UWORD bltshift = rorw(x & 15, 4);
  WORD n = src->depth;

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; dstmod -= 2;

    custom->bltalwm = 0;
    custom->bltamod = -2;
  } else {
    custom->bltalwm = -1;
    custom->bltamod = 0;
  }

  custom->bltdmod = dstmod;
  custom->bltcon0 = (SRCA | DEST | A_TO_D) | bltshift;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;

  for (; --n >= 0; srcbpt += src->bplSize, dstbpt += dst->bplSize) {
    WaitBlitter();

    custom->bltapt = srcbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  }
}


void BitmapCopyMasked(BitmapT *dst, UWORD x, UWORD y, BitmapT *src,
                      BitmapT *msk)
{
  APTR srcbpt = src->planes[0];
  APTR mskbpt = msk->planes[0];
  APTR dstbpt = dst->planes[0] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD dstmod = dst->bytesPerRow - src->bytesPerRow;
  UWORD bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  UWORD bltshift = rorw(x & 15, 4);
  WORD n = src->depth;

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; dstmod -= 2;

    custom->bltamod = -2;
    custom->bltbmod = -2;
    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC) | bltshift;
    custom->bltcon1 = bltshift;
    custom->bltalwm = 0;
    custom->bltafwm = -1;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;
  } else {
    custom->bltamod = 0;
    custom->bltbmod = 0;
    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NANBC);
    custom->bltcon1 = 0;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;
  }

  for (; --n >= 0; srcbpt += src->bplSize, dstbpt += dst->bplSize) {
    WaitBlitter();

    custom->bltapt = srcbpt;
    custom->bltbpt = mskbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  }
}

/* Restrictions: sx and sw must be multiply of 16! */
void BitmapCopyArea(BitmapT *dst, UWORD dx, UWORD dy, 
                    BitmapT *src, UWORD sx, UWORD sy, UWORD sw, UWORD sh)
{
  APTR srcbpt = src->planes[0] + ((sx & ~15) >> 3) + sy * src->bytesPerRow;
  APTR dstbpt = dst->planes[0] + ((dx & ~15) >> 3) + dy * dst->bytesPerRow;
  UWORD srcmod = src->bytesPerRow - (sw >> 3);
  UWORD dstmod = dst->bytesPerRow - (sw >> 3);
  UWORD bltsize = (sh << 6) | (sw >> 4);
  WORD n = src->depth;

  WaitBlitter();

  custom->bltamod = srcmod;
  custom->bltdmod = dstmod;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltalwm = -1;
  custom->bltafwm = -1;

  for (; --n >= 0; srcbpt += src->bplSize, dstbpt += dst->bplSize) {
    WaitBlitter();

    custom->bltapt = srcbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  }
}
