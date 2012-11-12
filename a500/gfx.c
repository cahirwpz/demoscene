#include <exec/memory.h>
#include <proto/exec.h>

#include "gfx.h"

__regargs BitmapT *NewBitmap(UWORD width, UWORD height, UWORD depth) {
  BitmapT *bitmap = AllocMem(sizeof(BitmapT) + depth * sizeof(APTR),
                             MEMF_PUBLIC|MEMF_CLEAR);

  bitmap->width = width;
  bitmap->height = height;
  bitmap->bytesPerRow = (width + 7) / 8;
  bitmap->depth = depth;

  {
    APTR *planePtr;
    APTR planes;
    LONG bplSize;

    /* Let's make it aligned to WORD boundary. */
    bplSize = bitmap->bytesPerRow * height;
    bplSize += bplSize & 1;

    planes = AllocMem((UWORD)bplSize * depth, MEMF_CHIP|MEMF_CLEAR);
    planePtr = bitmap->planes;

    do {
      *planePtr++ = planes;
      planes += bplSize;
    } while (--depth);

    bitmap->bplSize = bplSize;
  }

  return bitmap;
}

__regargs void DeleteBitmap(BitmapT *bitmap) {
  FreeMem(bitmap->planes, bitmap->bplSize * bitmap->depth);
  FreeMem(bitmap, sizeof(BitmapT) + bitmap->depth * (UWORD)sizeof(APTR));
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
