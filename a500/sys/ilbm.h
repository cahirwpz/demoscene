#ifndef __ILBM_H__
#define __ILBM_H__

#include "gfx.h"

__regargs BitmapT *LoadILBM(const char *filename, BOOL interleaved);
__regargs PaletteT *LoadPalette(const char *filename);

#endif
