#include "blitter.h"
#include "gfx.h"

/* Bitplane incrementer with saturation. */
__regargs void BitmapIncSaturated(BitmapT *dst_bm, BitmapT *carry_bm) {
  APTR carry0 = carry_bm->planes[0];
  APTR carry1 = carry_bm->planes[1];
  APTR *dst = dst_bm->planes;
  APTR ptr;
  UWORD bltsize = (dst_bm->height << 6) | (dst_bm->bytesPerRow >> 1);
  WORD n = dst_bm->depth;

  /* Only pixels set to one in carry[0] will be incremented. */
  
  WaitBlitter();
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    ptr = *dst++;

    WaitBlitter();
    custom->bltapt = ptr;
    custom->bltbpt = carry0;
    custom->bltdpt = carry1;
    custom->bltcon0 = HALF_ADDER_CARRY;
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltapt = ptr;
    custom->bltbpt = carry0;
    custom->bltdpt = ptr;
    custom->bltcon0 = HALF_ADDER;
    custom->bltsize = bltsize;

    swapr(carry0, carry1);
  }

  dst = dst_bm->planes;
  n = dst_bm->depth;
  while (--n >= 0) {
    ptr = *dst++;

    WaitBlitter();
    custom->bltapt = ptr;
    custom->bltbpt = carry0;
    custom->bltdpt = ptr;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltsize = bltsize;
  }
}
