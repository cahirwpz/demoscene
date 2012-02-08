#ifndef __SYSTEM_DISPLAY_H__
#define __SYSTEM_DISPLAY_H__

#include <graphics/gfx.h>

#include "gfx/palette.h"

typedef struct BitMap BitMapT;
typedef struct RastPort RastPortT;

BitMapT *GetCurrentBitMap();
RastPortT *GetCurrentRastPort();

void LoadPalette(PaletteT *palette);

void WaitForSafeToWrite();
void WaitForSafeToSwap();
void DisplaySwap();

bool InitDisplay(int width, int height, int depth);
void KillDisplay();

#endif
