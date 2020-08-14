#include <blitter.h>

typedef struct {
  const BitmapT *src;
  const BitmapT *msk;
  const BitmapT *dst;
  u_int start;
  u_short size;
} StateT;

static StateT state[1];

void BlitterCopyMaskedSetup(const BitmapT *dst, u_short x, u_short y,
                            const BitmapT *src, const BitmapT *msk)
{
  u_short dstmod = dst->bytesPerRow - src->bytesPerRow;
  u_short bltsize = (src->height << 6) | (src->bytesPerRow >> 1);
  u_short bltshift = rorw(x & 15, 4);

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

void BlitterCopyMaskedStart(short dstbpl, short srcbpl) {
  void *srcbpt = state->src->planes[srcbpl];
  void *dstbpt = state->dst->planes[dstbpl] + state->start;
  void *mskbpt = state->msk->planes[0];
  u_short bltsize = state->size;

  WaitBlitter();

  custom->bltapt = srcbpt;
  custom->bltbpt = mskbpt;
  custom->bltcpt = dstbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = bltsize;
}

void BitmapCopyMasked(const BitmapT *dst, u_short x, u_short y,
                      const BitmapT *src, const BitmapT *msk) 
{
  short i, n = min(dst->depth, src->depth);

  BlitterCopyMaskedSetup(dst, x, y, src, msk);
  for (i = 0; i < n; i++)
    BlitterCopyMaskedStart(i, i);
}
