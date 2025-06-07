#ifndef __SOFT_SPRITE_H__
#define __SOFT_SPRITE_H__

#include <types.h>
#include <sprite.h>
#include <system/memory.h>

typedef struct SoftSprite {
  short x, y;
  SpriteT *spr;
} SoftSpriteT;

/*
 * SprChanT represents data attached to single sprite DMA channel.
 */
typedef struct SprChan {
  SprDataT *curr;
  short length;
  short lastY; /* initially -1 */
  SprDataT word[__FLEX_ARRAY];
} SprChanT;

static int SoftSpriteCompare(const SoftSpriteT *s1, const SoftSpriteT *s2) {
  if (s1->y < s2->y)
    return -1;
  if (s1->y > s2->y)
    return 1;
  if (s1->x < s2->x)
    return -1;
  if (s1->x > s2->x)
    return 1;
  return 0;
}

static void SoftSpriteSort(SoftSpriteT *arr, short n) {
  short i, j;

  for (i = 1; i < n; i++) {
    SoftSpriteT key = arr[i];
    j = i - 1;

    /*
     * Move elements of arr[0..i-1], that are greater than `tmp`,
     * to one position ahead of their current position.
     */
    while (j >= 0 && SoftSpriteCompare(&arr[j], &key) < 0) {
      arr[j + 1] = arr[j];
      j = j - 1;
    }
    arr[j + 1] = key; // Place key in its correct position
  }
}

static void SprChanReset(SprChanT *chan) {
  chan->curr = chan->word;
  chan->lastY = -1;
  chan->word[0][0] = 0;
  chan->word[0][1] = 0;
}

SprChanT *AllocSprChan(int lines) {
  SprChanT *chan = MemAlloc(sizeof(SprChanT) + lines * sizeof(SprDataT), MEMF_CHIP);
  chan->length = lines;
  SprChanReset(chan);
  return chan;
}

static void ResetAllSprChan(SprChanT *chans) {
  short n = 8;

  while (--n >= 0) {
    SprChanReset(chans++);
  }
}

static SprChanT *AcquireSprChan(SprChanT *chans, short y, short h) {
  short n = 8;

  while (--n >= 0) {
    SprChanT *chan = chans++;
    if (chan->lastY < y) {
      chan->lastY = y + h + 1;
      return chan;
    }
  }

  return NULL;
}

static void FinishAllSprChan(SprChanT *chans) {
  short n = 8;

  while (--n >= 0) {
    SprChanT *chan = chans++;
    EndSprite(&chan->curr);
  }
}

static void AppendSprite(SpriteT *dst, short y, short h, SpriteT *src) {
  u_int *s = (u_int *)&src->data[y];
  u_int *d = (u_int *)&dst->data[0];

  while (--h >= 0) {
    *d++ = *s++;
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

void RenderSprites(SprChanT *chans, SoftSpriteT *swsprs, int n) {
  SoftSpriteSort(swsprs, n);
  ResetAllSprChan(chans);

  while (--n >= 0) {
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

    spr = MakeSprite(&chan->curr, sh, false);
    AppendSprite(spr, skip, sh, swspr->spr);
    SpriteUpdatePos(spr, X(sx), Y(sy));
  }

  FinishAllSprChan(chans);
}

#endif
