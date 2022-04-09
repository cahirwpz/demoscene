#include <sprite.h>

void MakeSprite(SprDataT **datp, u_short height, SpriteT *spr) {
  SprDataT *dat = *datp;
  dat->pos = SPRPOS(0, 0),
  dat->ctl = SPRCTL(0, 0, 0, height),
  spr->sprdat = dat;
  spr->height = height;
  *datp = (SprDataT *)&dat->data[height];
}
