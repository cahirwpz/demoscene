#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "coplist.h"
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
__regargs SpriteT *CloneSystemPointer();
__regargs void DeleteSprite(SpriteT *sprite);

/* Don't call these for null sprites. */
__regargs void UpdateSprite(SpriteT *sprite);
__regargs void UpdateSpritePos(SpriteT *sprite, UWORD hstart, UWORD vstart);

static inline void CopMakeSprites(CopListT *list, CopInsT **sprptr, SpriteT *nullspr) {
  UWORD i;

  for (i = 0; i < 8; i++)
    sprptr[i] = CopMove32(list, sprpt[i], nullspr->data);
}

static inline void CopMakeManualSprites(CopListT *list, CopInsT **sprptr, SpriteT *nullspr) {
  UWORD i;

  for (i = 0; i < 8; i++) {
    sprptr[i] = CopMove16(list, spr[i].pos, nullspr->data[0]);
    CopMove16(list, spr[i].ctl, nullspr->data[1]);
    CopMove32(list, sprpt[i], &nullspr->data[2]);
  }
}

#endif
