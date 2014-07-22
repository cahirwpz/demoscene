#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <exec/types.h>

typedef struct Color {
  UBYTE r, g, b;
} ColorT;

typedef struct Palette {
  UWORD count;
  ColorT colors[0];
} PaletteT;

__regargs PaletteT *NewPalette(UWORD count);
__regargs void DeletePalette(PaletteT *palette);

#define BM_INTERLEAVED 1

typedef struct Bitmap {
  UWORD width;
  UWORD height;
  UWORD depth;
  UWORD bytesPerRow;
  UWORD bplSize;
  BOOL  interleaved;
  PaletteT *palette;
  APTR  planes[7];
} BitmapT;

__regargs BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth,
                             BOOL interleaved);
__regargs void DeleteBitmap(BitmapT *bitmap);


#endif
