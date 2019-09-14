#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "coplist.h"
#include "gfx.h"

typedef struct Sprite SpriteT;

struct Sprite {
  SpriteT *attached;
  u_short height;
  u_short *data;
};

extern SpriteT *NullSprite;

__regargs SpriteT *NewSprite(u_short height, bool attached);
__regargs SpriteT *NewSpriteFromBitmap(u_short height, BitmapT *bitmap,
                                       u_short xstart, u_short ystart);
__regargs SpriteT *CloneSystemPointer(void);
__regargs void DeleteSprite(SpriteT *sprite);

/* Don't call it for null sprites. */
__regargs void UpdateSprite(SpriteT *sprite, u_short hstart, u_short vstart);

__regargs void CopSetupSprites(CopListT *list, CopInsT **sprptr);
__regargs void CopSetupManualSprites(CopListT *list, CopInsT **sprptr);

#endif
