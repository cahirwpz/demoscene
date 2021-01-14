#include <sprite.h>
#include <memory.h>

SpriteT *NewSprite(u_short height, bool attached) {
  SpriteT *sprite = MemAlloc(attached ? 2 * sizeof(SpriteT) : sizeof(SpriteT),
                             MEMF_PUBLIC|MEMF_CLEAR);
  int size = (height + 2) * 4;

  sprite[0].attached = attached;
  sprite[0].height = height;
  sprite[0].data = MemAlloc(size, MEMF_CHIP|MEMF_CLEAR);

  if (attached) {
    sprite[1].height = height;
    sprite[1].data = MemAlloc(size, MEMF_CHIP|MEMF_CLEAR);
  }

  return sprite;
}
