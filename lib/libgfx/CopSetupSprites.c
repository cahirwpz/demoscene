#include <sprite.h>

CopInsPairT *CopSetupSprites(CopListT *list) {
  SpriteT *spr = NullSprData;
  CopInsPairT *sprptr = CopInsPtr(list);
  short n = 8;
  int i = 0;

  while (--n >= 0)
    CopMove32(list, sprpt[i++], spr);

  return sprptr;
}
