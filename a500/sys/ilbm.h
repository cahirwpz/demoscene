#ifndef __ILBM_H__
#define __ILBM_H__

#include "gfx.h"

__regargs void BitmapUnpack(BitmapT *bitmap, UWORD flags);
__regargs BitmapT *LoadILBMCustom(CONST STRPTR filename, UWORD flags);
__regargs PaletteT *LoadPalette(CONST STRPTR filename);

static inline BitmapT *LoadILBM(CONST STRPTR filename) {
  return LoadILBMCustom(filename, BM_DISPLAYABLE|BM_LOAD_PALETTE);
}

#endif
