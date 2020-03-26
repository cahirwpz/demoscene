#include "memory.h"
#include "gfx.h"

__regargs void InitSharedBitmap(BitmapT *bitmap, u_short width, u_short height,
                                u_short depth, BitmapT *donor)
{
  u_short bytesPerRow = ((width + 15) & ~15) / 8;

  bitmap->width = width;
  bitmap->height = height;
  bitmap->depth = depth;
  bitmap->bytesPerRow = bytesPerRow;
  bitmap->bplSize = bytesPerRow * height;
  bitmap->flags = donor->flags;
  bitmap->palette = donor->palette;

  BitmapSetPointers(bitmap, donor->planes[0]);
}

__regargs void BitmapSetPointers(BitmapT *bitmap, void *planes) {
  int modulo =
    (bitmap->flags & BM_INTERLEAVED) ? bitmap->bytesPerRow : bitmap->bplSize;
  short depth = bitmap->depth;
  void **planePtr = bitmap->planes;

  do {
    *planePtr++ = planes;
    planes += modulo;
  } while (depth--);
}

__regargs BitmapT *NewBitmapCustom(u_short width, u_short height, u_short depth,
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
    u_int size = BitmapSize(bitmap);
    void *planes = MemAlloc(size, MEMF_CHIP);

    memcpy(planes, bitmap->planes[0], size - BM_EXTRA);
    MemFree(bitmap->planes[0]);

    bitmap->flags |= BM_DISPLAYABLE;
    BitmapSetPointers(bitmap, planes);
  }
}

__regargs PaletteT *NewPalette(u_short count) {
  PaletteT *palette = MemAlloc(sizeof(PaletteT) + count * sizeof(ColorT),
                               MEMF_PUBLIC|MEMF_CLEAR);
  palette->count = count;
  return palette;
}

__regargs void DeletePalette(PaletteT *palette) {
  if (palette)
    MemFree(palette);
}

__regargs void ConvertPaletteToRGB4(PaletteT *palette, u_short *color, short n) {
  u_char *src = (u_char *)palette->colors;

  if (palette->count < n)
    n = palette->count;

  while (--n >= 0) {
    u_char r = *src++ & 0xf0;
    u_char g = *src++ & 0xf0;
    u_char b = *src++ & 0xf0;
    *color++ = (r << 4) | (u_char)(g | (b >> 4));
  }
}

__regargs bool ClipBitmap(const Box2D *space, Point2D *pos, Area2D *area) {
  short minX = space->minX;
  short minY = space->minY;
  short maxX = space->maxX;
  short maxY = space->maxY;
  short posX = pos->x;
  short posY = pos->y;

  if ((posX + area->w <= minX) || (posX > maxX))
    return false;
  if ((posY + area->h <= minY) || (posY > maxY))
    return false;

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

  return true;
}

__regargs bool InsideArea(short x, short y, Area2D *area) {
  short x1 = area->x;
  short y1 = area->y;
  short x2 = area->x + area->w - 1;
  short y2 = area->y + area->h - 1;

  return (x1 <= x && x <= x2 && y1 <= y && y <= y2);
}
