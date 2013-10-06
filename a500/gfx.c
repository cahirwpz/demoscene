#include <exec/memory.h>
#include <proto/exec.h>

#include "gfx.h"

__regargs BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth,
                             BOOL interleaved)
{
  BitmapT *bitmap = AllocMem(sizeof(BitmapT), MEMF_PUBLIC|MEMF_CLEAR);

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

    planes = AllocMem((UWORD)bplSize * depth, MEMF_CHIP|MEMF_CLEAR);
    planePtr = bitmap->planes;

    if (interleaved)
      bplSize = width / 8;

    do {
      *planePtr++ = planes;
      planes += bplSize;
    } while (--depth);
  }

  return bitmap;
}

__regargs void DeleteBitmap(BitmapT *bitmap) {
  FreeMem(bitmap->planes[0], bitmap->bplSize * bitmap->depth);
  FreeMem(bitmap, sizeof(BitmapT));
}

__regargs PaletteT *NewPalette(UWORD count) {
  PaletteT *palette = AllocMem(sizeof(PaletteT) + count * sizeof(ColorT),
                               MEMF_PUBLIC|MEMF_CLEAR);
  palette->count = count;

  return palette;
}

__regargs void DeletePalette(PaletteT *palette) {
  FreeMem(palette, sizeof(PaletteT) + palette->count * sizeof(ColorT));
}
