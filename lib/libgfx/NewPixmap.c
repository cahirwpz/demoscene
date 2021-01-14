#include <memory.h>
#include <pixmap.h>

static short bitdepth[] = { 0, 4, 8 };

static int PixmapSize(PixmapT *pixmap) {
  short bitsPerPixel = bitdepth[pixmap->type & PM_DEPTH_MASK];
  short bytesPerRow;
  if (pixmap->type & _PM_RGB) {
    bitsPerPixel = (bitsPerPixel * 3 + 7) >> 3;
    bytesPerRow = bitsPerPixel * pixmap->width;
  } else {
    bytesPerRow = (bitsPerPixel * pixmap->width + 7) >> 3;
  }
  return bytesPerRow * pixmap->height;
}

PixmapT *NewPixmap(short width, short height, PixmapTypeT type,
                   u_int memoryAttributes)
{
  PixmapT *pixmap = MemAlloc(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = MemAlloc(PixmapSize(pixmap), memoryAttributes);

  return pixmap;
}
