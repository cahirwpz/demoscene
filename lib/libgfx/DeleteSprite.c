#include <sprite.h>
#include <memory.h>

void DeleteSprite(SpriteT *spr) {
  MemFree(spr);
}
