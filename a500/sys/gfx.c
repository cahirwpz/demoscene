#include "memory.h"
#include "gfx.h"

__regargs void InitSharedBitmap(BitmapT *bitmap, UWORD width, UWORD height,
                                UWORD depth, BitmapT *donor)
{
  bitmap->width = width;
  bitmap->height = height;
  bitmap->depth = depth;
  bitmap->bytesPerRow = (width + 7) / 8;
  bitmap->bplSize = bitmap->bytesPerRow * height;
  bitmap->interleaved = donor->interleaved;
  bitmap->palette = donor->palette;

  ITER(i, 0, depth - 1, bitmap->planes[i] = donor->planes[i]);
}

__regargs BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth,
                             BOOL interleaved)
{
  BitmapT *bitmap = MemAlloc(sizeof(BitmapT), MEMF_PUBLIC|MEMF_CLEAR);

  bitmap->width = width;
  bitmap->height = height;
  bitmap->bytesPerRow = (width + 7) / 8;
  bitmap->depth = depth;
  bitmap->interleaved = interleaved;

  {
    APTR *planePtr;
    APTR planes;
    LONG bplSize;

    /* Let's make it aligned to WORD boundary. */
    bplSize = bitmap->bytesPerRow * height;
    bplSize += bplSize & 1;

    bitmap->bplSize = bplSize;

    /* Allocate extra two bytes for scratchpad area.
     * Used by blitter line drawing. */
    planes = MemAlloc((UWORD)bplSize * depth + 2, MEMF_CHIP|MEMF_CLEAR);
    planePtr = bitmap->planes;

    if (interleaved)
      bplSize = width / 8;

    do {
      *planePtr++ = planes;
      planes += bplSize;
    } while (depth--);
  }

  return bitmap;
}

__regargs void DeleteBitmap(BitmapT *bitmap) {
  MemFree(bitmap->planes[0], bitmap->bplSize * bitmap->depth + 2);
  MemFree(bitmap, sizeof(BitmapT));
}

__regargs PaletteT *NewPalette(UWORD count) {
  PaletteT *palette = MemAlloc(sizeof(PaletteT) + count * sizeof(ColorT),
                               MEMF_PUBLIC|MEMF_CLEAR);
  palette->count = count;

  return palette;
}

__regargs PaletteT *CopyPalette(PaletteT *palette) {
  PaletteT *copy = NewPalette(palette->count);
  memcpy(copy->colors, palette->colors, palette->count * sizeof(ColorT));
  return copy;
}

__regargs void DeletePalette(PaletteT *palette) {
  MemFree(palette, sizeof(PaletteT) + palette->count * sizeof(ColorT));
}
