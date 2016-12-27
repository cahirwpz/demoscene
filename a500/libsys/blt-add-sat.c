#include "blitter.h"

/* Bitplane adder with saturation. */
void BitmapAddSaturated(BitmapT *dst_bm, WORD dx, WORD dy, BitmapT *src_bm, BitmapT *carry_bm) {
  ULONG dst_begin = ((dx & ~15) >> 3) + dy * (WORD)dst_bm->bytesPerRow;
  UWORD dst_modulo = (dst_bm->bytesPerRow - src_bm->bytesPerRow) - 2;
  UWORD src_shift = rorw(dx & 15, 4);
  UWORD bltsize = ((src_bm->height << 6) | (src_bm->bytesPerRow >> 1)) + 1;
  APTR carry0 = carry_bm->planes[0];
  APTR carry1 = carry_bm->planes[1];
  APTR *src = src_bm->planes;
  APTR *dst = dst_bm->planes;

  {
    APTR aptr = (*src++);
    APTR bptr = (*dst++) + dst_begin;

    WaitBlitter();

    /* Initialize blitter */
    custom->bltamod = -2;
    custom->bltbmod = dst_modulo;
    custom->bltcmod = 0;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = 0;

    /* Bitplane 0: half adder with carry. */
    custom->bltapt = aptr;
    custom->bltbpt = bptr;
    custom->bltdpt = carry0;
    custom->bltdmod = 0;
    custom->bltcon0 = HALF_ADDER_CARRY | src_shift;
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltapt = aptr;
    custom->bltbpt = bptr;
    custom->bltdpt = bptr;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = HALF_ADDER | src_shift;
    custom->bltsize = bltsize;
  }

  {
    WORD n = src_bm->depth - 1;

    /* Bitplane 1-n: full adder with carry. */
    while (--n >= 0) {
      APTR aptr = (*src++);
      APTR bptr = (*dst++) + dst_begin;

      WaitBlitter();
      custom->bltapt = aptr;
      custom->bltbpt = bptr;
      custom->bltcpt = carry0;
      custom->bltdpt = carry1;
      custom->bltdmod = 0;
      custom->bltcon0 = FULL_ADDER_CARRY | src_shift;
      custom->bltsize = bltsize;

      WaitBlitter();
      custom->bltapt = aptr;
      custom->bltbpt = bptr;
      custom->bltcpt = carry0;
      custom->bltdpt = bptr;
      custom->bltdmod = dst_modulo;
      custom->bltcon0 = FULL_ADDER | src_shift;
      custom->bltsize = bltsize;

      swapr(carry0, carry1);
    }
  }

  /* Apply saturation bits. */
  {
    WORD n = src_bm->depth;

    dst = dst_bm->planes;

    WaitBlitter();
    custom->bltamod = dst_modulo;
    custom->bltbmod = 0;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltalwm = -1;

    while (--n >= 0) {
      APTR bptr = (*dst++) + dst_begin;

      WaitBlitter();
      custom->bltapt = bptr;
      custom->bltbpt = carry0;
      custom->bltdpt = bptr;
      custom->bltsize = bltsize;
    }
  }
}
