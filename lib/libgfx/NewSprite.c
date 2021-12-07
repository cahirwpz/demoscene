#include <sprite.h>
#include <memory.h>

SpriteT *NewSprite(u_short height, bool attached) {
  int size = (height + 2) * sizeof(u_short) * 2;

  SpriteT *spr = MemAlloc(size, MEMF_CHIP|MEMF_CLEAR);
  spr->pos = SPRPOS(0, 0);
  spr->ctl = SPRCTL(0, 0, attached, height);

  return spr;
}
