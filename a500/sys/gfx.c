#include "memory.h"
#include "gfx.h"

__regargs void InitSharedBitmap(BitmapT *bitmap, UWORD width, UWORD height,
                                UWORD depth, BitmapT *donor)
{
  bitmap->width = width;
  bitmap->height = height;
  bitmap->depth = depth;
  bitmap->bytesPerRow = ((width + 15) & ~15) / 8;
  bitmap->bplSize = bitmap->bytesPerRow * height;
  bitmap->flags = donor->flags;
  bitmap->palette = donor->palette;

  BitmapSetPointers(bitmap, donor->planes[0]);
}

__regargs void BitmapSetPointers(BitmapT *bitmap, APTR planes) {
  LONG modulo =
    (bitmap->flags & BM_INTERLEAVED) ? bitmap->bytesPerRow : bitmap->bplSize;
  WORD depth = bitmap->depth;
  APTR *planePtr = bitmap->planes;

  do {
    *planePtr++ = planes;
    planes += modulo;
  } while (depth--);
}

__regargs BitmapT *NewBitmapCustom(UWORD width, UWORD height, UWORD depth,
                                   UBYTE flags)
{
  BitmapT *bitmap = MemAlloc(sizeof(BitmapT), MEMF_PUBLIC|MEMF_CLEAR);
  UWORD bytesPerRow = ((width + 15) & ~15) / 8;

  bitmap->width = width;
  bitmap->height = height;
  bitmap->bytesPerRow = bytesPerRow;
  /* Let's make it aligned to WORD boundary. */
  bitmap->bplSize = bytesPerRow * height;
  bitmap->depth = depth;
  bitmap->flags = flags & BM_FLAGMASK;

  if (!(flags & BM_MINIMAL)) {
    ULONG memoryFlags = 0;

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

__regargs void DeleteBitmap(BitmapT *bitmap) {
  if (!(bitmap->flags & BM_MINIMAL)) {
    if (bitmap->compression)
      MemFree(bitmap->planes[0], (LONG)bitmap->planes[1]);
    else
      MemFree(bitmap->planes[0], BitmapSize(bitmap));
  }
  MemFree(bitmap, sizeof(BitmapT));
}

__regargs void BitmapMakeDisplayable(BitmapT *bitmap) {
  if (!(bitmap->flags & BM_DISPLAYABLE) && (bitmap->compression == COMP_NONE)) {
    ULONG size = BitmapSize(bitmap);
    APTR planes = MemAlloc(size, MEMF_CHIP);

    memcpy(planes, bitmap->planes[0], size - BM_EXTRA);
    MemFree(bitmap->planes[0], size);

    bitmap->flags |= BM_DISPLAYABLE;
    BitmapSetPointers(bitmap, planes);
  }
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
