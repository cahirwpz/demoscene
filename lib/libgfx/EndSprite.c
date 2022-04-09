#include <sprite.h>

void EndSprite(SprDataT **datp) {
  u_int *dat = (u_int *)*datp;
  *dat++ = 0;
  *datp = (SprDataT *)dat;
}
