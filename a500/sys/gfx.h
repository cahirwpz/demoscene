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

/* Size of extra buffer after last word of bitplanes. */
#define BM_EXTRA sizeof(UWORD)

/* Flags stored in Bitmap structure. */
#define BM_CLEAR        1
#define BM_DISPLAYABLE  2
#define BM_INTERLEAVED  4
#define BM_MINIMAL      8
#define BM_HAM         16
#define BM_FLAGMASK    31

/* Flags than can be passed to functions that load Bitmap. */
#define BM_LOAD_PALETTE 16
#define BM_KEEP_PACKED  32

/* Bitplane compression method. */
#define COMP_NONE      0
#define COMP_RLE       1
#define COMP_DEFLATE 254
#define COMP_LZO     255

typedef struct Bitmap {
  UWORD width;
  UWORD height;
  UWORD depth;
  UWORD bytesPerRow;
  UWORD bplSize;
  UBYTE flags;
  UBYTE compression;
  PaletteT *palette;
  APTR  planes[7];
} BitmapT;

__regargs void InitSharedBitmap(BitmapT *bitmap, UWORD width, UWORD height,
                                UWORD depth, BitmapT *donor);
__regargs void BitmapSetPointers(BitmapT *bitmap, APTR planes);

__regargs BitmapT *NewBitmapCustom(UWORD width, UWORD height, UWORD depth,
                                   UBYTE flags);
__regargs void DeleteBitmap(BitmapT *bitmap);
__regargs void BitmapMakeDisplayable(BitmapT *bitmap);

static inline BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth) {
  return NewBitmapCustom(width, height, depth, BM_CLEAR|BM_DISPLAYABLE);
}

static inline ULONG BitmapSize(BitmapT *bitmap) {
  /* Allocate extra two bytes for scratchpad area.
   * Used by blitter line drawing. */
  return ((UWORD)bitmap->bplSize * (UWORD)bitmap->depth) + BM_EXTRA;
}

#endif
