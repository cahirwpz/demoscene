#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "common.h"

typedef struct Palette {
  u_short count;
  u_short colors[0];
} PaletteT;

typedef struct {
  short x, y;
} Point2D;

typedef struct {
  short x1, y1;
  short x2, y2;
} Line2D;

typedef struct {
  short minX, minY;
  short maxX, maxY;
} Box2D;

typedef struct {
  short w, h;
} Size2D;

typedef struct {
  short x, y;
  short w, h;
} Area2D;

__regargs PaletteT *NewPalette(u_short count);
__regargs void DeletePalette(PaletteT *palette);

/* Size of extra buffer after last word of bitplanes. */
#define BM_EXTRA sizeof(u_short)

/* Flags stored in Bitmap structure. */
#define BM_CLEAR        0x01
#define BM_DISPLAYABLE  0x02
#define BM_INTERLEAVED  0x04
#define BM_MINIMAL      0x08
#define BM_HAM          0x10
#define BM_EHB          0x20
#define BM_STATIC       0x40 /* bitplanes were allocated statically */
#define BM_FLAGMASK     0x7F

/* Maximum number of bitplanes kept in bitmap structure. */
#define BM_NPLANES 7

typedef struct Bitmap {
  u_short width;
  u_short height;
  u_short depth;
  u_short bytesPerRow;
  u_short bplSize;
  u_char flags;
  void *planes[BM_NPLANES];
} BitmapT;

__regargs void InitSharedBitmap(BitmapT *bitmap, u_short width, u_short height,
                                u_short depth, BitmapT *donor);

__regargs BitmapT *NewBitmapCustom(u_short width, u_short height, u_short depth,
                                   u_char flags);
__regargs void DeleteBitmap(BitmapT *bitmap);
__regargs void BitmapMakeDisplayable(BitmapT *bitmap);
__regargs bool ClipArea(const Box2D *space, Point2D *pos, Area2D *area);
__regargs bool InsideArea(short x, short y, Area2D *area);

static inline BitmapT *NewBitmap(u_short width, u_short height, u_short depth) {
  return NewBitmapCustom(width, height, depth, BM_CLEAR|BM_DISPLAYABLE);
}

#endif
