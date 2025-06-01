#include <sprite.h>

SprDataT *MakeSprite(SprDataT **datp, short height, bool attached) {
  SprDataT *dat = *datp;
  *datp = (SprDataT *)&dat->data[height];
  dat->pos = SPRPOS(0, 0);
  dat->ctl = SPRCTL(0, 0, attached, height);
  return dat;
}
