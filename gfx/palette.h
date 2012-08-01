#ifndef __GFX_PALETTE_H__
#define __GFX_PALETTE_H__

#include "std/types.h"
#include "gfx/colors.h"

typedef struct Palette {
  RGB *colors;
  size_t start;
  size_t count;
  struct Palette *next;
} PaletteT;

PaletteT *NewPalette(size_t count);
PaletteT *NewPaletteFromFile(const StrT fileName);

bool LinkPalettes(PaletteT *palette, ...);
void UnlinkPalettes(PaletteT *palette);

#endif
