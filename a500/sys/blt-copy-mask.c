#include "blitter.h"

typedef struct {
  BitmapT *src;
  BitmapT *dst;
  BitmapT *msk;
  ULONG start;
  UWORD size;
} StateT;

static StateT state[1];

__regargs void BlitterCopyMaskedSetup(BitmapT *dst, UWORD x, UWORD y,
                                      BitmapT *src, BitmapT *msk)
{
  UWORD dstmod = dst->bytesPerRow - src->bytesPerRow;
  UWORD bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  UWORD bltshift = rorw(x & 15, 4);

  state->src = src;
  state->dst = dst;
  state->msk = msk;
  state->start = ((x & ~15) >> 3) + y * dst->bytesPerRow;

  if (bltshift)
    bltsize++, dstmod -= 2;

  WaitBlitter();

  if (bltshift) {
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
}

__regargs void BlitterCopyMaskedStart(WORD dstbpl, WORD srcbpl) {
  APTR srcbpt = state->src->planes[srcbpl];
  APTR dstbpt = state->dst->planes[dstbpl] + state->start;
  APTR mskbpt = state->msk->planes[0];
  UWORD bltsize = state->size;

  WaitBlitter();

  custom->bltapt = srcbpt;
  custom->bltbpt = mskbpt;
  custom->bltcpt = dstbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

void BitmapCopyMasked(BitmapT *dst, UWORD x, UWORD y,
                      BitmapT *src, BitmapT *msk) 
{
  WORD i, n = min(dst->depth, src->depth);

  BlitterCopyMaskedSetup(dst, x, y, src, msk);
  for (i = 0; i < n; i++)
    BlitterCopyMaskedStart(i, i);
}
