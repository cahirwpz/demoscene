#include <proto/exec.h>

#include "memory.h"
#include "pixmap.h"

static inline WORD BitsPerPixel(PixmapTypeT type) {
  if (type == PM_RGB4)
    return 16;
  if ((type == PM_GRAY4) || (type == PM_CMAP4))
    return 4;
  return 8;
}

__regargs PixmapT *NewPixmap(UWORD width, UWORD height, 
                             PixmapTypeT type, ULONG memoryAttributes)
{
  PixmapT *pixmap = MemAlloc(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);
  WORD bytesPerRow = width * BitsPerPixel(type) >> 3;

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = MemAlloc(bytesPerRow * height, memoryAttributes);

  return pixmap;
}

__regargs PixmapT *ClonePixmap(PixmapT *pixmap) {
  PixmapT *clone = NewPixmap(pixmap->width, pixmap->height,
                            pixmap->type, TypeOfMem(pixmap->pixels));
  WORD bytesPerRow = pixmap->width * BitsPerPixel(pixmap->type) >> 3;

  memcpy(clone->pixels, pixmap->pixels, bytesPerRow * pixmap->height);

  return clone;
}

__regargs void DeletePixmap(PixmapT *pixmap) {
  WORD bytesPerRow = pixmap->width * BitsPerPixel(pixmap->type) >> 3;

  MemFree(pixmap->pixels, bytesPerRow * pixmap->height);
  MemFree(pixmap, sizeof(PixmapT));
}
