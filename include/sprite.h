#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "copper.h"
#include "gfx.h"

/*
 * An array of `SprData` placed in chip memory is a simple abstraction
 * over data fed into sprite DMA channel.
 *
 * The data structure for DMA channel may consist of several sprite data
 * terminated by control word filled with zeros.
 *
 * +======+======+ <- sprite 0
 * |  POS |  CTL |
 * +------+------+
 * | DATA | DATB |
 * | .... | .... |
 * | DATA | DATB |
 * +======+======+ <- sprite 1
 * | .... | .... |
 * +======+======+
 * | .... | .... |
 * +======+======+ <- sprite N-1
 * |  POS |  CTL |
 * +------+------+
 * | DATA | DATB |
 * | .... | .... |
 * | DATA | DATB |
 * +======+======+ <- terminator
 * |    0 |    0 |
 * +======+======+
 *
 * You could organize continuous chunk of memory into several DMA channel as
 * well. Data for another channel would begin just after a terminator.
 */

typedef u_short SprDataT[2];

typedef struct Sprite {
  u_short pos;
  u_short ctl;
  SprDataT data[__FLEX_ARRAY];
} SpriteT;

/*
 * SPRxPOS:
 *  Bits 15-8 contain the low 8 bits of VSTART
 *  Bits 7-0 contain the high 8 bits of HSTART
 */
#define SPRPOS(X, Y) (u_short)(((Y) << 8) | (((X) >> 1) & 255))

/*
 * SPRxCTL:
 *  Bits 15-8       The low eight bits of VSTOP
 *  Bit 7           (Used in attachment)
 *  Bits 6-3        Unused (make zero)
 *  Bit 2           The VSTART high bit
 *  Bit 1           The VSTOP high bit
 *  Bit 0           The HSTART low bit
 */
#define SPRCTL(X, Y, A, H)                                                     \
  (u_short)(                                                                   \
   ((u_short)(((Y) + (H))) << 8) |                                             \
   ((A) ? 0x80 : 0) |                                                          \
   (((Y) >> 6) & 4) |                                                          \
   (((u_short)((Y) + (H)) >> 7) & 2) |                                         \
   ((X) & 1))

#define SPREND() ((SprDataT){0, 0})

extern SpriteT NullSprData[];

/*
 * Calculates space for sprite data to be fed into DMA channel.
 * `height` is total number of pixel lines and `nctrl` number of control words.
 */
static inline int SprDataSize(u_short height, u_short nctrl) {
  return (height + nctrl) * sizeof(u_int);
}

/* Determines height of the sprite based on `pos` and `ctl` words. */
short SpriteHeight(SpriteT *spr);

/*
 * Consumes space for `pos`, `ctr` and `height` long words of pixel data
 * from `dat` to construct storage for sprite data.
 *
 * Marks sprite as attached if `attached` is set to true.
 *
 * `datp` will point to next usable sprite data (possibly uninitialized).
 * You should call MakeSprite or EndSprite on this value.
 *
 * Returns pointer to first control word of the sprite.
 */
SpriteT *MakeSprite(SprDataT **datp, short height, bool attached);

/*
 * Terminate sprite data for DMA channel by writing zero long word after
 * last long word of pixel data.
 */
void EndSprite(SprDataT **datp);

static inline void SpriteSetHeader(SpriteT *spr, short hs, short vs, bool attached, short height) {
  u_char *raw = (u_char *)spr;

  *raw++ = vs;
  *raw++ = (u_short)hs >> 1;

  {
    u_short vstop = vs + height;
    u_char lowctl = hs & 1;

    *raw++ = vstop;

    if (attached)
      lowctl += 0x80;
    if (vs >= 0x100)
      lowctl += 4;
    if (vstop >= 0x100)
      lowctl += 2;
    *raw++ = lowctl;
  }
}

/* Don't call it for null sprites. */
void SpriteUpdatePos(SpriteT *spr, hpos hstart, vpos vstart);

CopInsPairT *CopSetupSprites(CopListT *list);

void ResetSprites(void);

static inline void CopInsSetSprite(CopInsPairT *sprptr, SpriteT *spr) {
  CopInsSet32(sprptr, spr);
}

#endif
