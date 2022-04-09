#include <sprite.h>

void CopSetupManualSprites(CopListT *list, CopInsT **sprptr) {
  SprDataT *spr = NullSprData;
  short i;

  for (i = 0; i < 8; i++) {
    sprptr[i] = CopMove16(list, spr[i].pos, spr->pos);
    CopMove16(list, spr[i].ctl, spr->ctl);
    CopMove32(list, sprpt[i], spr->data);
  }
}
