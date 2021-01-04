#include <sprite.h>

void CopSetupManualSprites(CopListT *list, CopInsT **sprptr) {
  u_short *data = NullSprite;
  short i;

  for (i = 0; i < 8; i++) {
    sprptr[i] = CopMove16(list, spr[i].pos, data[0]);
    CopMove16(list, spr[i].ctl, data[1]);
    CopMove32(list, sprpt[i], data + 2);
  }
}
