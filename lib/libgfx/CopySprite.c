#include <sprite.h>
#include <string.h>
#include <system/memory.h>

/* TODO: does not handle split sprites */
void CopySprite(SpriteT *copy, SpriteT *orig) {
  int size = SprDataSize(orig->height, 2);
  copy->sprdat = MemAlloc(size, MEMF_CHIP);
  copy->height = orig->height;
  copy->attached = orig->attached;
  memcpy(copy->sprdat, orig->sprdat, size);
}
