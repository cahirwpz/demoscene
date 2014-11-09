#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "common.h"

typedef struct Color {
  UBYTE r, g, b;
} ColorT;

typedef struct Palette {
  UWORD count;
  ColorT colors[0];
} PaletteT;

typedef struct {
  WORD x1, y1;
  WORD x2, y2;
} Line2D;

__regargs PaletteT *NewPalette(UWORD count);
__regargs PaletteT *CopyPalette(PaletteT *palette);
__regargs void DeletePalette(PaletteT *palette);

#define BM_INTERLEAVED 1

typedef struct Bitmap {
  UWORD width;
  UWORD height;
  UWORD depth;
  UWORD bytesPerRow;
  UWORD bplSize;
  ULONG flags;
  PaletteT *palette;
  APTR  planes[7];
} BitmapT;

__regargs void InitSharedBitmap(BitmapT *bitmap, UWORD width, UWORD height,
                                UWORD depth, BitmapT *donor);

__regargs BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth);
__regargs void DeleteBitmap(BitmapT *bitmap);

#endif
