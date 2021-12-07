#include <sprite.h>

void CopSetupSprites(CopListT *list, CopInsT **sprptr) {
  SpriteT *spr = NullSprite;
  short i;

  for (i = 0; i < 8; i++) {
    CopInsT *ins = CopMove32(list, sprpt[i], spr);
    if (sprptr)
      *sprptr++ = ins;
  }
}
