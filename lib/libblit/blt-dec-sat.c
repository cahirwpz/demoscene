#include "blitter.h"

/* Bitplane decrementer with saturation. */
void BitmapDecSaturated(const BitmapT *dst_bm, const BitmapT *borrow_bm) {
  void *borrow0 = borrow_bm->planes[0];
  void *borrow1 = borrow_bm->planes[1];
  void *const *dst = dst_bm->planes;
  void *ptr = *dst++;
  u_short bltsize = (dst_bm->height << 6) | (dst_bm->bytesPerRow >> 1);
  short n = dst_bm->depth - 1;

  WaitBlitter();
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbdat = -1;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltapt = ptr;
  custom->bltdpt = borrow0;
  custom->bltcon0 = HALF_SUB_BORROW & ~SRCB;
  custom->bltsize = bltsize;

  WaitBlitter();
  custom->bltapt = ptr;
  custom->bltdpt = ptr;
  custom->bltcon0 = HALF_SUB & ~SRCB;
  custom->bltsize = bltsize;

  while (--n >= 0) {
    ptr = *dst++;

    WaitBlitter();
    custom->bltapt = ptr;
    custom->bltbpt = borrow0;
    custom->bltdpt = borrow1;
    custom->bltcon0 = HALF_SUB_BORROW;
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltapt = ptr;
    custom->bltbpt = borrow0;
    custom->bltdpt = ptr;
    custom->bltcon0 = HALF_SUB;
    custom->bltsize = bltsize;

    swapr(borrow0, borrow1);
  }

  dst = dst_bm->planes;
  n = dst_bm->depth;
  while (--n >= 0) {
    ptr = *dst++;

    WaitBlitter();
    custom->bltapt = ptr;
    custom->bltbpt = borrow0;
    custom->bltdpt = ptr;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_AND_NOT_B;
    custom->bltsize = bltsize;
  }
}
