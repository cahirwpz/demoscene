#ifndef __SOFT_SPRITE_H__
#define __SOFT_SPRITE_H__

#include <types.h>
#include <sprite.h>
#include <blitter.h>
#include <system/memory.h>

typedef struct SoftSprite {
  hpos hp;
  vpos vp;
  SpriteT *spr;
} SoftSpriteT;

/*
 * SprChanT represents data attached to single sprite DMA channel.
 */
typedef struct SprChan {
  short lastVP; /* initially -1 */
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
    while ((prev >= first) && (prev->vp.vpos > this.vp.vpos))
      *curr-- = *prev--;
    *curr = this;
  }
}

static inline void SprChanReset(SprChanT *chan) {
  chan->curr = (SprDataT *)chan->spr;
  chan->lastVP = -1;
  *(u_int *)chan->spr = 0;
}

static void InitSprChan(SprChanT *chan, int lines) {
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

static inline SprChanT *AcquireSprChan(SprChanT *chan, short vp, short height) {
  short n = 8;

  while (--n >= 0) {
    if (chan->lastVP < vp) {
      chan->lastVP = vp + height + 1;
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

static void AppendSpriteBlitter(SprDataT *dst, SprDataT *src, short height) {
  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltapt = src;
  custom->bltdpt = dst;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltsize = (height << 6) | 2;
}

static inline void SpriteSetHeader(SpriteT *spr, short hs, short vs, short height) {
  u_char *raw = (u_char *)spr;

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

typedef void (*AppendSpriteT)(SprDataT *dst, SprDataT *src, short h);

typedef struct SoftSpriteArray {
  short count;
  AppendSpriteT append;
  SoftSpriteT *last;
  SoftSpriteT *sprites;
} SoftSpriteArrayT;

#define SOFTSPRITEARRAY(NAME, SIZE)         \
  SoftSpriteArrayT NAME = {                 \
    .count = 0,                             \
    .append = AppendSpriteBlitter,          \
    .last = NULL,                           \
    .sprites = (SoftSpriteT[SIZE]) {}       \
  }

static inline void SoftSpriteArrayReset(SoftSpriteArrayT *array) {
  array->count = 0;
  array->last = array->sprites;
}

static inline void AddSprite(SoftSpriteArrayT *array, SpriteT *spr, hpos hp, vpos vp) {
  SoftSpriteT *swspr = array->last++;

  swspr->spr = spr;
  swspr->hp = hp;
  swspr->vp = vp;

  array->count++;
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

static void RenderSprites(SprChanT chans[8], SoftSpriteArrayT *array, DispWinT *diw) {
  SoftSpriteT *sprites = array->sprites;
  AppendSpriteT append = array->append;
  short n = array->count;

  SoftSpriteSort(sprites, n);
  ResetAllSprChan(chans);

  do {
    SoftSpriteT *swspr = sprites++;
    SprChanT *chan;

    short hs = swspr->hp.hpos;
    short vs = swspr->vp.vpos;
    short skip = 0;
    short height = SpriteHeight(swspr->spr);

    /* left edge */
    if (hs + 16 <= diw->left.hpos)
      continue;
    /* right edge */
    if (hs >= diw->right.hpos)
      continue;
    /* top edge */
    if (vs < diw->top.vpos) {
      skip = diw->top.vpos - vs;
      height -= skip;
      vs += skip;
    }
    /* bottom edge */
    if (vs + height >= diw->bottom.vpos)
      height = diw->bottom.vpos - vs;
    if (height <= 0)
      continue;

    if (!(chan = AcquireSprChan(chans, vs, height)))
      continue;

    /* Allocate space for sprite */
    {
      SpriteT *spr = (SpriteT *)chan->curr;
      chan->curr = &spr->data[height];

      append(&spr->data[0], &swspr->spr->data[skip], height);
      SpriteSetHeader(spr, hs, vs, height);
    }
  } while (--n > 0);

  FinishAllSprChan(chans);
}

#endif
