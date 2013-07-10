#include "gfx/colorfunc.h"
#include "std/memory.h"

uint8_t *NewColorFunc() {
  return NewTable(uint8_t, 256);
}

void ColorFuncLevels(uint8_t *cfunc, int no_levels, uint8_t *levels) {
  int i, c;

  for (i = 0, c = 0; i < 256; i++) {
    if (c < no_levels && i == levels[c])
      c++;
    cfunc[i] = c;
  }
}
