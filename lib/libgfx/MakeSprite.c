#include <sprite.h>

SpriteT *MakeSprite(SprDataT **datp, short height, bool attached) {
  SpriteT *spr = (SpriteT *)*datp;
  *datp = &spr->data[height];
  spr->pos = SPRPOS(0, 0);
  spr->ctl = SPRCTL(0, 0, attached, height);
  return spr;
}
