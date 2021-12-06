#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "copper.h"
#include "gfx.h"

typedef struct Sprite {
  u_short pos;
  u_short ctl;
  u_short data[0];
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

extern SpriteT NullSpriteData;

#define NullSprite (&NullSpriteData)

SpriteT *NewSprite(u_short height, bool attached);
void DeleteSprite(SpriteT *spr);

/* Don't call it for null sprites. */
void SpriteUpdatePos(SpriteT *spr, u_short height,
                     u_short hstart, u_short vstart);

static inline void SpriteSetAttached(SpriteT *spr) {
  spr->ctl |= 0x80;
}

void CopSetupSprites(CopListT *list, CopInsT **sprptr);
void CopSetupManualSprites(CopListT *list, CopInsT **sprptr);

#endif
