#include "bltop.h"

typedef struct {
  BitmapT *src;
  BitmapT *dst;
  ULONG start;
  UWORD size;
} StateT;

static StateT state[1];

__regargs void BlitterFastCopySetup(BitmapT *dst, UWORD x, UWORD y,
                                    BitmapT *src) 
{
  UWORD dstmod = dst->bytesPerRow - src->bytesPerRow;
  UWORD bltshift = rorw(x & 15, 4);
  UWORD bltsize = (src->height << 6) | (src->bytesPerRow >> 1);

  if (bltshift)
    bltsize++, dstmod -= 2;

  state->src = src;
  state->dst = dst;
  state->start = ((x & ~15) >> 3) + y * dst->bytesPerRow;
  state->size = bltsize;

  WaitBlitter();

  if (bltshift) {
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
}

__regargs void BlitterFastCopyStart(WORD dstbpl, WORD srcbpl) {
  APTR srcbpt = state->src->planes[srcbpl];
  APTR dstbpt = state->dst->planes[dstbpl] + state->start;
  UWORD bltsize = state->size;

  WaitBlitter();

  custom->bltapt = srcbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

__regargs void BitmapCopyFast(BitmapT *dst, UWORD x, UWORD y, BitmapT *src) {
  WORD i, n = min(dst->depth, src->depth);

  BlitterFastCopySetup(dst, x, y, src);
  for (i = 0; i < n; i++)
    BlitterFastCopyStart(i, i);
}
