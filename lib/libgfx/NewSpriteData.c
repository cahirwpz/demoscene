#include <sprite.h>
#include <memory.h>

SprDataT *NewSpriteData(u_short height, u_short nctrl) {
  int size = (height + nctrl) * sizeof(u_int);

  return MemAlloc(size, MEMF_CHIP|MEMF_CLEAR);
}
