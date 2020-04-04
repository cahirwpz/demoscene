#include "memory.h"
#include "gfx.h"

__regargs PaletteT *NewPalette(u_short count) {
  PaletteT *palette = MemAlloc(sizeof(PaletteT) + count * sizeof(u_short),
                               MEMF_PUBLIC|MEMF_CLEAR);
  palette->count = count;
  return palette;
}

__regargs void DeletePalette(PaletteT *palette) {
  if (palette)
    MemFree(palette);
}
