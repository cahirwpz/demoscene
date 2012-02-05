#ifndef __GFX_PALETTE_H__
#define __GFX_PALETTE_H__

#include "std/types.h"
#include "gfx/common.h"

typedef struct Palette {
  ColorT *colors;
  size_t num;
} PaletteT;

PaletteT *NewPalette(size_t num);
PaletteT *NewPaletteFromFile(const char *fileName, uint32_t memFlags);
void DeletePalette(PaletteT *palette);

#endif
