#ifndef __GFX_PALETTE_H__
#define __GFX_PALETTE_H__

#include "std/types.h"
#include "gfx/common.h"

typedef struct Palette {
  ColorT *colors;
  size_t start;
  size_t count;
  struct Palette *next;
} PaletteT;

PaletteT *NewPalette(size_t count);
PaletteT *NewPaletteFromFile(const StrT fileName);
PaletteT *CopyPalette(PaletteT *palette);

bool LinkPalettes(PaletteT *palette, ...);
void UnlinkPalettes(PaletteT *palette);

#endif
