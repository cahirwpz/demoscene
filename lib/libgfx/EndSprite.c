#include <sprite.h>

void EndSprite(SprDataT **datp) {
  SprDataT *dat = *datp;
  dat->pos = 0;
  dat->ctl = 0;
  *datp = (SprDataT *)dat->data;
}
