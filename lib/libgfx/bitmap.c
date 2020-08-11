#include <string.h>
#include "memory.h"
#include "gfx.h"

static inline u_int BitmapSize(BitmapT *bitmap) {
  /* Allocate extra two bytes for scratchpad area.
   * Used by blitter line drawing. */
  return ((u_short)bitmap->bplSize * (u_short)bitmap->depth) + BM_EXTRA;
}

static void BitmapSetPointers(BitmapT *bitmap, void *planes) {
  int modulo =
    (bitmap->flags & BM_INTERLEAVED) ? bitmap->bytesPerRow : bitmap->bplSize;
  short depth = bitmap->depth;
  void **planePtr = bitmap->planes;

  do {
    *planePtr++ = planes;
    planes += modulo;
  } while (depth--);
}

void InitSharedBitmap(BitmapT *bitmap, u_short width, u_short height,
                      u_short depth, BitmapT *donor)
{
  u_short bytesPerRow = ((width + 15) & ~15) / 8;

  bitmap->width = width;
  bitmap->height = height;
  bitmap->depth = depth;
  bitmap->bytesPerRow = bytesPerRow;
  bitmap->bplSize = bytesPerRow * height;
  bitmap->flags = donor->flags;

  BitmapSetPointers(bitmap, donor->planes[0]);
}

BitmapT *NewBitmapCustom(u_short width, u_short height, u_short depth,
                         u_char flags)
{
  BitmapT *bitmap = MemAlloc(sizeof(BitmapT), MEMF_PUBLIC|MEMF_CLEAR);
  u_short bytesPerRow = ((width + 15) & ~15) / 8;

  bitmap->width = width;
  bitmap->height = height;
  bitmap->bytesPerRow = bytesPerRow;
  /* Let's make it aligned to short boundary. */
  bitmap->bplSize = bytesPerRow * height;
  bitmap->depth = depth;
  bitmap->flags = flags & BM_FLAGMASK;

  if (!(flags & BM_MINIMAL)) {
    u_int memoryFlags = 0;

    /* Recover memory flags. */
    if (flags & BM_CLEAR)
      memoryFlags |= MEMF_CLEAR;

    if (flags & BM_DISPLAYABLE)
      memoryFlags |= MEMF_CHIP;
    else
      memoryFlags |= MEMF_PUBLIC;

    BitmapSetPointers(bitmap, MemAlloc(BitmapSize(bitmap), memoryFlags));
  }

  return bitmap;
}

void DeleteBitmap(BitmapT *bitmap) {
  if (bitmap) {
    if (!(bitmap->flags & BM_MINIMAL))
      MemFree(bitmap->planes[0]);
    MemFree(bitmap);
  }
}

void BitmapMakeDisplayable(BitmapT *bitmap) {
  if (!(bitmap->flags & BM_DISPLAYABLE)) {
    u_int size = BitmapSize(bitmap);
    void *planes = MemAlloc(size, MEMF_CHIP);

    memcpy(planes, bitmap->planes[0], size - BM_EXTRA);
    MemFree(bitmap->planes[0]);

    bitmap->flags |= BM_DISPLAYABLE;
    BitmapSetPointers(bitmap, planes);
  }
}
