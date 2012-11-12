#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <exec/types.h>

typedef struct Bitmap {
  UWORD width;
  UWORD height;
  UWORD bytesPerRow;
  UWORD bplSize;
  UWORD depth;
  UWORD pad;
  APTR  planes[0];
} BitmapT;

__regargs BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth);
__regargs void DeleteBitmap(BitmapT *bitmap);

typedef struct Color {
  UBYTE r, g, b;
} ColorT;

typedef struct Palette {
  UWORD count;
  ColorT colors[0];
} PaletteT;

__regargs PaletteT *NewPalette(UWORD count);
__regargs void DeletePalette(PaletteT *palette);

#endif
