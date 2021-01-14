#include <sprite.h>
#include <memory.h>

void DeleteSprite(SpriteT *sprite) {
  if (sprite) {
    if (sprite->attached)
      MemFree(sprite[1].data);

    MemFree(sprite->data);
    MemFree(sprite);
  }
}
