#ifndef __SOFT_SPRITE_H__
#define __SOFT_SPRITE_H__

#include <types.h>
#include <stdlib.h>
#include <sprite.h>
#include <blitter.h>
#include <system/memory.h>

typedef struct SoftSprite {
  short x, y;
  SpriteT *spr;
} SoftSpriteT;

/*
 * SprChanT represents data attached to single sprite DMA channel.
 */
typedef struct SprChan {
  short lastY; /* initially -1 */
  short length;
  SprDataT *curr;
  SpriteT *spr;
} SprChanT;

static void SoftSpriteSort(SoftSpriteT *first, short n) {
  SoftSpriteT *ptr = &first[1];
  SoftSpriteT *last = &first[n-1];

  while (ptr <= last) {
    SoftSpriteT *curr = ptr;
    SoftSpriteT *prev = ptr - 1;
    SoftSpriteT this = *ptr++;
    while ((prev >= first) && (prev->y > this.y))
      *curr-- = *prev--;
    *curr = this;
  }
}

static inline void SprChanReset(SprChanT *chan) {
  chan->curr = (SprDataT *)chan->spr;
  chan->lastY = -1;
  *(u_int *)chan->spr = 0;
}

void InitSprChan(SprChanT *chan, int lines) {
  chan->spr = MemAlloc(lines * sizeof(SprDataT), MEMF_CHIP);
  chan->length = lines;
  SprChanReset(chan);
}

static inline void ResetAllSprChan(SprChanT *chan) {
  short n = 8;

  while (--n >= 0) {
    SprChanReset(chan++);
  }
}

static inline SprChanT *AcquireSprChan(SprChanT *chan, short y, short h) {
  short n = 8;

  while (--n >= 0) {
    if (chan->lastY < y) {
      chan->lastY = y + h + 1;
      return chan;
    }
    chan++;
  }

  return NULL;
}

static inline void FinishAllSprChan(SprChanT *chan) {
  short n = 8;

  while (--n >= 0) {
    *(u_int *)chan->curr = 0;
    chan++;
  }
}

static inline void AppendSprite(SpriteT *dst, short y, short h, SpriteT *src) {
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltapt = &src->data[y];
  custom->bltdpt = &dst->data[0];
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltsize = (h << 6) | 2;
  WaitBlitter();
}

static inline void SpriteSetHeader(SpriteT *spr, hpos hstart, vpos vstart, short height) {
  u_char *raw = (u_char *)spr;
  short hs = hstart.hpos;
  short vs = vstart.vpos;

  *raw++ = vs;
  *raw++ = (u_short)hs >> 1;

  {
    u_short vstop = vs + height;
    u_char lowctl = hs & 1;

    *raw++ = vstop;

    if (vs >= 0x100)
      lowctl += 4;
    if (vstop >= 0x100)
      lowctl += 2;
    *raw++ = lowctl;
  }
}

/*
 * For simplicity it's assumed that no Copper based HW trick will be used.
 * Moreover all sprites are 3-colors in the same palette. Input bitmaps are
 * interleaved and width is divisable by 16.
 *
 * 1. Sort SoftSprite by Y then X.
 * 2. Initialize all 8 SpriteT to empty.
 * 3. For each SoftSprite, let it be SRC do:
 *    a) determine visible height SH starting at SY
 *    b) if SH <= 0, then skip
 *    c) for each 16-pixel wide stripe at SX of the SoftSprite:
 *       - if outside of the screen, then skip
 *       - find usable SprChanT, let it be DST
 *         - usable sprite must have lastY > Y
 *       - if no DST, then skip
 *       - allocate SH+1 lines of DST (MakeSprite)
 *       - copy with Blitter SH+1 lines from the SRC[SX,SY,SX+16,SY+SH] to DST
 *       - DST.lastY += SH+1
 * 4. Terminate all SprChan (EndSprite)
 */

void RenderSprites(SprChanT chans[8], SoftSpriteT *swsprs, int n) {
  short i;

  SoftSpriteSort(swsprs, n);
  ResetAllSprChan(chans);

  for (i = 0; i < n; i++) {
    SoftSpriteT *swspr = swsprs++;
    SprChanT *chan;
    SpriteT *spr;

    short sx = swspr->x;
    short sy = swspr->y;
    short skip = 0;
    short sh = SpriteHeight(swspr->spr);

    /* left edge */
    if (sx + 16 <= 0)
      continue;
    /* right edge */
    if (sx >= WIDTH)
      continue;
    /* top edge */
    if (sy < 0) {
      sh += sy;
      skip -= sy;
      sy = 0;
    }
    /* bottom edge */
    if (sy + sh >= HEIGHT)
      sh -= sy + sh - HEIGHT;
    if (sh <= 0)
      continue;

    if (!(chan = AcquireSprChan(chans, sy, sh)))
      continue;

    /* Allocate space for sprite */
    spr = (SpriteT *)chan->curr;
    chan->curr = &spr->data[sh];

    AppendSprite(spr, skip, sh, swspr->spr);
    SpriteSetHeader(spr, X(sx), Y(sy), sh);
  }

  FinishAllSprChan(chans);
}

#endif
