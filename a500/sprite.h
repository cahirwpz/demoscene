#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "gfx.h"

typedef struct Sprite SpriteT;

struct Sprite {
  SpriteT *attached;
  UWORD height;
  UWORD x, y;
  UWORD *data;
};

__regargs SpriteT *NewSprite(UWORD height, BOOL attached);
__regargs SpriteT *NewSpriteFromBitmap(UWORD height, BitmapT *bitmap,
                                       UWORD xstart, UWORD ystart);
__regargs void DeleteSprite(SpriteT *sprite);
__regargs void UpdateSprite(SpriteT *sprite);
__regargs void UpdateSpritePos(SpriteT *sprite, UWORD hstart, UWORD vstart);

#endif
