#include <blitter.h>

typedef struct {
  const BitmapT *src;
  const BitmapT *dst;
  u_int src_start;
  u_int dst_start;
  u_short size;
  bool fast;
} StateT;

static StateT state[1];

#define sx area->x
#define sy area->y
#define sw area->w
#define sh area->h

#define START(x) (((x) & ~15) >> 3)
#define ALIGN(x) START((x) + 15)

/* This routine assumes following conditions:
 *  - there's always enough space in `dst` to copy area from `src`
 *  - at least one of `sx` and `dx` is aligned to word boundary 
 */
void BlitterCopyAreaSetup(const BitmapT *dst, u_short dx, u_short dy,
                          const BitmapT *src, const Area2D *area)
{
  u_short dxo = dx & 15;
  u_short sxo = sx & 15;
  bool forward = dxo >= sxo;
  u_short xo = forward ? dxo : sxo;
  u_short width = xo + sw;
  u_short wo = width & 15;
  u_short bytesPerRow = ALIGN(width);
  u_short srcmod = src->bytesPerRow - bytesPerRow;
  u_short dstmod = dst->bytesPerRow - bytesPerRow;
  u_short bltafwm = FirstWordMask[dxo];
  u_short bltalwm = LastWordMask[wo];
  u_short bltshift = rorw(xo, 4);

  /*
   * TODO: Two cases exist where number of word for 'src' and 'dst' differ.
   * 1. (sxo < (sxo + area->w) & 15) && (dxo > (dxo + area->w) & 15)
   * 2. (sxo > (sxo + area->w) & 15) && (dxo < (dxo + area->w) & 15)
   * To handle them an in-memory mask must be created as suggested by HRM:
   * http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0121.html
   */

  state->src = src;
  state->dst = dst;
  state->src_start = START(sx);
  state->dst_start = START(dx);
  state->size = (sh << 6) | (bytesPerRow >> 1);

  if (forward) {
    state->src_start += (short)sy * (short)src->bytesPerRow;
    state->dst_start += (short)dy * (short)dst->bytesPerRow;
  } else {
    state->src_start += (short)(sy + sh - 1) * (short)src->bytesPerRow
                      + bytesPerRow - 2;
    state->dst_start += (short)(dy + sh - 1) * (short)dst->bytesPerRow
                      + bytesPerRow - 2;
  }

  state->fast = (xo == 0) && (wo == 0);

  WaitBlitter();

  if (!state->fast) {
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | NABC | ABNC | NANBC);
    custom->bltcon1 = bltshift | (forward ? 0 : BLITREVERSE);
    custom->bltadat = -1;
    if (forward) {
      custom->bltafwm = bltafwm;
      custom->bltalwm = bltalwm;
    } else {
      custom->bltafwm = bltalwm;
      custom->bltalwm = bltafwm;
    }
    custom->bltbmod = srcmod;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;
  } else {
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = -1;
    custom->bltamod = srcmod;
    custom->bltdmod = dstmod;
  }
}

void BlitterCopyAreaStart(short dstbpl, short srcbpl) {
  void *srcbpt = state->src->planes[srcbpl] + state->src_start;
  void *dstbpt = state->dst->planes[dstbpl] + state->dst_start;
  u_short bltsize = state->size;

  if (state->fast) {
    WaitBlitter();

    custom->bltapt = srcbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  } else {
    WaitBlitter();

    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  }
}
