#include "memory.h"
#include "pixmap.h"

static inline UWORD BitsPerPixel(PixmapTypeT type) {
  if (type == PM_RGB4)
    return 16;
  if (type == PM_GRAY4)
    return 4;
  return 8;
}

__regargs PixmapT *NewPixmap(UWORD width, UWORD height, 
                             PixmapTypeT type, ULONG memoryAttributes)
{
  PixmapT *pixmap = AllocMemSafe(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = AllocMemSafe(width * height * BitsPerPixel(type) >> 3,
                                memoryAttributes);

  return pixmap;
}

__regargs void DeletePixmap(PixmapT *pixmap) {
  FreeMem(pixmap->pixels,
          pixmap->width * pixmap->height * BitsPerPixel(pixmap->type) >> 3);
  FreeMem(pixmap, sizeof(PixmapT));
}
