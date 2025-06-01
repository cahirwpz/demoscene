#include <sprite.h>

short SpriteHeight(SprDataT *sprdat) {
  u_char *raw = (u_char *)sprdat;
  short vstart, vstop, height;

  vstart = *raw++;
  raw++;
  vstop = *raw++;

  height = vstop - vstart;
  if (*raw & 4)
    height -= 0x100;
  if (*raw & 2)
    height += 0x100;

  return height;
}

