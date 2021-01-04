#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "common.h"

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

u_int BitmapSize(BitmapT *bitmap);
void BitmapSetPointers(BitmapT *bitmap, void *planes);

void InitSharedBitmap(BitmapT *bitmap, u_short width, u_short height,
                      u_short depth, BitmapT *donor);

BitmapT *NewBitmapCustom(u_short width, u_short height, u_short depth,
                         u_char flags);
void DeleteBitmap(BitmapT *bitmap);
void BitmapMakeDisplayable(BitmapT *bitmap);

static inline BitmapT *NewBitmap(u_short width, u_short height, u_short depth) {
  return NewBitmapCustom(width, height, depth, BM_CLEAR|BM_DISPLAYABLE);
}

#endif
