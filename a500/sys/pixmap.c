#include <proto/exec.h>

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
  PixmapT *pixmap = MemAlloc(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = MemAlloc(width * height * BitsPerPixel(type) >> 3,
                            memoryAttributes);

  return pixmap;
}

__regargs PixmapT *ClonePixmap(PixmapT *pixmap) {
  PixmapT *clone = NewPixmap(pixmap->width, pixmap->height,
                            pixmap->type, TypeOfMem(pixmap->pixels));

  memcpy(clone->pixels, pixmap->pixels,
         pixmap->width * pixmap->height * BitsPerPixel(pixmap->type) >> 3);

  return clone;
}

__regargs void DeletePixmap(PixmapT *pixmap) {
  MemFree(pixmap->pixels,
          pixmap->width * pixmap->height * BitsPerPixel(pixmap->type) >> 3);
  MemFree(pixmap, sizeof(PixmapT));
}
