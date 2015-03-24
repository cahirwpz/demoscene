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
  WORD x, y;
} Point2D;

typedef struct {
  WORD x1, y1;
  WORD x2, y2;
} Line2D;

typedef struct {
  WORD minX, minY;
  WORD maxX, maxY;
} Box2D;

typedef struct {
  WORD x, y;
  WORD w, h;
} Area2D;

__regargs PaletteT *NewPalette(UWORD count);
__regargs PaletteT *CopyPalette(PaletteT *palette);
__regargs void DeletePalette(PaletteT *palette);
__regargs void ConvertPaletteToRGB4(PaletteT *palette, UWORD *color, WORD n);

/* Size of extra buffer after last word of bitplanes. */
#define BM_EXTRA sizeof(UWORD)

/* Flags stored in Bitmap structure. */
#define BM_CLEAR        0x01
#define BM_DISPLAYABLE  0x02
#define BM_INTERLEAVED  0x04
#define BM_MINIMAL      0x08
#define BM_HAM          0x10
#define BM_EHB          0x20
#define BM_FLAGMASK     0x3F

/* Flags than can be passed to functions that load Bitmap. */
#define BM_LOAD_PALETTE 0x40
#define BM_KEEP_PACKED  0x80

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
  ULONG pchgTotal;
  UWORD *pchg;
  APTR  planes[7];
} BitmapT;

__regargs void InitSharedBitmap(BitmapT *bitmap, UWORD width, UWORD height,
                                UWORD depth, BitmapT *donor);
__regargs void BitmapSetPointers(BitmapT *bitmap, APTR planes);

__regargs BitmapT *NewBitmapCustom(UWORD width, UWORD height, UWORD depth,
                                   UBYTE flags);
__regargs void DeleteBitmap(BitmapT *bitmap);
__regargs void BitmapMakeDisplayable(BitmapT *bitmap);
__regargs BOOL ClipBitmap(const Box2D *space, Point2D *pos, Area2D *area);

static inline BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth) {
  return NewBitmapCustom(width, height, depth, BM_CLEAR|BM_DISPLAYABLE);
}

static inline ULONG BitmapSize(BitmapT *bitmap) {
  /* Allocate extra two bytes for scratchpad area.
   * Used by blitter line drawing. */
  return ((UWORD)bitmap->bplSize * (UWORD)bitmap->depth) + BM_EXTRA;
}

#endif
