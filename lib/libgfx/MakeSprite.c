#include <sprite.h>

void MakeSprite(SprDataT **datp, u_int height, bool attached, SpriteT *spr) {
  SprDataT *dat = *datp;
  *datp = (SprDataT *)&dat->data[height];
  spr->sprdat = dat;
  spr->height = height;
  spr->attached = attached;
  dat->pos = SPRPOS(0, 0);
  dat->ctl = SPRCTL(0, 0, attached, height);
}
