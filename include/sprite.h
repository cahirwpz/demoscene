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
 * Obviously you could place several DMA channel continuously in memory.
 */

typedef struct SprData {
  u_short pos;
  u_short ctl;
  u_short data[0][2];
} SprDataT;

typedef struct Sprite {
  SprDataT *data;
  u_short height;
} SpriteT;

/*
 * SPRxPOS:
 *  Bits 15-8 contain the low 8 bits of VSTART
 *  Bits 7-0 contain the high 8 bits of HSTART
 */
#define SPRPOS(X, Y) (((Y) << 8) | (((X) >> 1) & 255))

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
  (((((Y) + (H) + 1) & 255) << 8) |                                            \
   (((A) & 1) << 7) |                                                          \
   (((Y) & 256) >> 6) |                                                        \
   ((((Y) + (H) + 1) & 256) >> 7) |                                            \
   ((X) & 1))

extern SprDataT NullSprData[];

/*
 * Allocates space for sprite data to be fed into DMA channel.
 * `height` is total number of pixel lines and `nctrl` number of control words.
 */
SprDataT *NewSprData(u_short height, u_short nctrl);
void DeleteSprData(SprDataT *sprdat);

/*
 * Consumes space for `pos`, `ctr` and `height` long words of pixel data
 * from `dat` to construct storage for sprite data.
 *
 * Information about sprite will be written back to `spr` structure.
 *
 * Returns a pointer to next usable sprite data (possibly uninitialized).
 * You should call MakeSprite or EndSprite on return value.
 */
SprDataT *MakeSprite(SpriteT *spr, SprDataT *dat, u_short height);

/*
 * Terminate sprite data for DMA channel by writing zero long word after
 * last long word of pixel data.
 */
SprDataT *EndSprite(SprDataT *dat);

/* Don't call it for null sprites. */
void SpriteUpdatePos(SpriteT *spr, u_short hstart, u_short vstart);

static inline void SpriteSetAttached(SpriteT *spr) {
  spr->data->ctl |= 0x80;
}

void CopSetupSprites(CopListT *list, CopInsT **sprptr);
void CopSetupManualSprites(CopListT *list, CopInsT **sprptr);

static inline void CopInsSetSprite(CopInsT *sprptr, SpriteT *spr) {
  CopInsSet32(sprptr, spr->data);
}

#endif
