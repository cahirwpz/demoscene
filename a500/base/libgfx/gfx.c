#include "memory.h"
#include "gfx.h"

__regargs void InitSharedBitmap(BitmapT *bitmap, UWORD width, UWORD height,
                                UWORD depth, BitmapT *donor)
{
  UWORD bytesPerRow = ((width + 15) & ~15) / 8;

  bitmap->width = width;
  bitmap->height = height;
  bitmap->depth = depth;
  bitmap->bytesPerRow = bytesPerRow;
  bitmap->bplSize = bytesPerRow * height;
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
  if (bitmap) {
    if (!(bitmap->flags & BM_MINIMAL))
      MemFree(bitmap->planes[0]);
    MemFree(bitmap->pchg);
    MemFree(bitmap);
  }
}

__regargs void BitmapMakeDisplayable(BitmapT *bitmap) {
  if (!(bitmap->flags & BM_DISPLAYABLE) && (bitmap->compression == COMP_NONE)) {
    ULONG size = BitmapSize(bitmap);
    APTR planes = MemAlloc(size, MEMF_CHIP);

    memcpy(planes, bitmap->planes[0], size - BM_EXTRA);
    MemFree(bitmap->planes[0]);

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
  if (palette)
    MemFree(palette);
}

__regargs void ConvertPaletteToRGB4(PaletteT *palette, UWORD *color, WORD n) {
  UBYTE *src = (UBYTE *)palette->colors;

  if (palette->count < n)
    n = palette->count;

  while (--n >= 0) {
    UBYTE r = *src++ & 0xf0;
    UBYTE g = *src++ & 0xf0;
    UBYTE b = *src++ & 0xf0;
    *color++ = (r << 4) | (UBYTE)(g | (b >> 4));
  }
}

void RotatePalette(PaletteT *dstpal, PaletteT *srcpal, WORD start, WORD end, WORD step) {
  ColorT *src = srcpal->colors;
  ColorT *dst = dstpal->colors + start;
  WORD n = end - start + 1;
  WORD s = mod16(step, n);
  WORD i = start + s;

  if (s < 0)
    i += n;

  while (--n >= 0) {
    *dst++ = src[i++];
    if (i > end)
      i = start;
  }
}

__regargs BOOL ClipBitmap(const Box2D *space, Point2D *pos, Area2D *area) {
  WORD minX = space->minX;
  WORD minY = space->minY;
  WORD maxX = space->maxX;
  WORD maxY = space->maxY;
  WORD posX = pos->x;
  WORD posY = pos->y;

  if ((posX + area->w <= minX) || (posX > maxX))
    return FALSE;
  if ((posY + area->h <= minY) || (posY > maxY))
    return FALSE;

  if (posX < minX) {
    area->x += minX - posX;
    area->w -= minX - posX;
    pos->x = posX = minX;
  }

  if (posY < minY) {
    area->y += minY - posY;
    area->h -= minY - posY;
    pos->y = posY = minY;
  }

  if (posX + area->w > maxX)
    area->w = maxX - posX + 1;

  if (posY + area->h > maxY)
    area->h = maxY - posY + 1;

  return TRUE;
}

__regargs BOOL InsideArea(WORD x, WORD y, Area2D *area) {
  WORD x1 = area->x;
  WORD y1 = area->y;
  WORD x2 = area->x + area->w - 1;
  WORD y2 = area->y + area->h - 1;

  return (x1 <= x && x <= x2 && y1 <= y && y <= y2);
}
