#ifndef __ILBM_H__
#define __ILBM_H__

#include "gfx.h"

__regargs void BitmapUnpack(BitmapT *bitmap, u_short flags);
__regargs BitmapT *LoadILBMCustom(const char *filename, u_short flags);

static inline BitmapT *LoadILBM(const char *filename) {
  return LoadILBMCustom(filename, BM_DISPLAYABLE|BM_LOAD_PALETTE);
}

#endif
