#include <sprite.h>
#include <memory.h>

void DeleteSpriteData(SprDataT *spr) {
  MemFree(spr);
}
