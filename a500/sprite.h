#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "gfx.h"

typedef struct Sprite {
  UWORD height;
  UWORD x, y;
  UWORD *data;
} SpriteT;

__regargs SpriteT *NewSprite(UWORD height);
__regargs SpriteT *NewSpriteFromBitmap(UWORD height, BitmapT *bitmap,
                                       UWORD xstart, UWORD ystart);
__regargs void DeleteSprite(SpriteT *sprite);
__regargs void UpdateSprite(SpriteT *sprite);

static inline void UpdateSpritePos(SpriteT *sprite, UWORD hstart, UWORD vstart) {
  sprite->x = hstart;
  sprite->y = vstart;
  UpdateSprite(sprite);
}

#endif
