#include <sprite.h>

void MakeSprite(SprDataT **datp, u_int height, SpriteT *spr) {
  SprDataT *dat = *datp;
  *datp = (SprDataT *)&dat->data[height];
  spr->sprdat = dat;
  spr->height = height;
  dat->pos = SPRPOS(0, 0);
  dat->ctl = SPRCTL(0, 0, 0, height);
}
