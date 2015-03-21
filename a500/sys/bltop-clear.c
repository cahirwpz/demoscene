#include "gfx.h"
#include "blitter.h"

__regargs void BitmapClear(BitmapT *dst_bm, WORD n) {
  UWORD bltsize = (dst_bm->height << 6) | (dst_bm->bytesPerRow >> 1);
  APTR *dst = dst_bm->planes;

  WaitBlitter();

  custom->bltadat = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;

  while (--n >= 0) {
    APTR ptr = *dst++;

    WaitBlitter();

    custom->bltdpt = ptr;
    custom->bltsize = bltsize;
  }
}

void BitmapClearArea(BitmapT *dst, WORD n, UWORD x, UWORD y, UWORD w, UWORD h) {
  APTR dstbpt = dst->planes[0] + ((x & ~15) >> 3) + y * dst->bytesPerRow;
  UWORD bltsize = (h << 6) | (w >> 4);
  UWORD bltmod = dst->bytesPerRow - (w >> 3);
  UWORD bltshift = rorw(x & 15, 4);

  WaitBlitter();

  if (bltshift) {
    bltsize += 1; bltmod -= 2; 

    custom->bltadat = 0xffff;
    custom->bltcdat = 0;
    custom->bltbmod = -2;
    custom->bltcon0 = (SRCB | DEST) | (NABC | NABNC | ABC | ANBC) | bltshift;
    custom->bltcon1 = bltshift;
    custom->bltdmod = bltmod;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
  } else {
    custom->bltadat = 0;
    custom->bltcon0 = DEST | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltdmod = bltmod;
    custom->bltalwm = -1;
    custom->bltafwm = -1;
  }

  for (; --n >= 0; dstbpt += dst->bplSize) {
    WaitBlitter();

    if (bltshift) {
      custom->bltbpt = dstbpt;
      custom->bltdpt = dstbpt;
      custom->bltsize = bltsize;
    } else {
      custom->bltdpt = dstbpt;
      custom->bltsize = bltsize;
    }
  }
}
