#ifndef __PALETTE_H__
#define __PALETTE_H__

#include "common.h"
#include "custom.h"

typedef struct Palette {
  u_short count;
  u_short colors[0];
} PaletteT;

void LoadPalette(const PaletteT *palette, u_int start);

static inline void SetColor(u_short i, u_short rgb) {
  custom->color[i] = rgb;
}

#endif
