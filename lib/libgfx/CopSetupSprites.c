#include <sprite.h>

void CopSetupSprites(CopListT *list, CopInsT **sprptr) {
  u_short *data = NullSprite;
  short i;

  for (i = 0; i < 8; i++) {
    CopInsT *ins = CopMove32(list, sprpt[i], data);
    if (sprptr)
      *sprptr++ = ins;
  }
}
