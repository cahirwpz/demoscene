#ifndef __GFX_PALETTE_H__
#define __GFX_PALETTE_H__

#include "std/types.h"
#include "gfx/colors.h"

typedef struct Palette PaletteT;

struct Palette {
  RGB *colors;
  uint16_t start;
  uint16_t count;
  PaletteT *next;
};

PaletteT *NewPalette(size_t count);
PaletteT *NewPaletteFromFile(const char *fileName);

bool LinkPalettes(PaletteT *palette, ...);
void UnlinkPalettes(PaletteT *palette);

int PaletteFindNearest(PaletteT *palette, RGB color);

void PaletteModify(PaletteT *dst, PaletteT *src, ColorModifyFuncT func);

#endif
