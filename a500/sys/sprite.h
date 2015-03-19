#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "coplist.h"
#include "gfx.h"

typedef struct Sprite SpriteT;

struct Sprite {
  SpriteT *attached;
  UWORD height;
  UWORD *data;
};

extern SpriteT *NullSprite;

__regargs SpriteT *NewSprite(UWORD height, BOOL attached);
__regargs SpriteT *NewSpriteFromBitmap(UWORD height, BitmapT *bitmap,
                                       UWORD xstart, UWORD ystart);
__regargs SpriteT *CloneSystemPointer();
__regargs void DeleteSprite(SpriteT *sprite);

/* Don't call it for null sprites. */
__regargs void UpdateSpritePos(SpriteT *sprite, UWORD hstart, UWORD vstart);

__regargs void CopMakeSprites(CopListT *list, CopInsT **sprptr);
__regargs void CopMakeManualSprites(CopListT *list, CopInsT **sprptr);

#endif
