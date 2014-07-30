#include "memory.h"
#include "pixmap.h"

__regargs PixmapT *NewPixmap(UWORD width, UWORD height, PixmapTypeT type) {
  PixmapT *pixmap = AllocMemSafe(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);
  UWORD pixelSize = (type == PM_RGB4) ? 2 : 1;

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = AllocMemSafe(width * height * pixelSize,
                                MEMF_PUBLIC|MEMF_CLEAR);

  return pixmap;
}

__regargs void DeletePixmap(PixmapT *pixmap) {
  UWORD pixelSize = (pixmap->type == PM_RGB4) ? 2 : 1;

  FreeMem(pixmap->pixels, pixmap->width * pixmap->height * pixelSize);
  FreeMem(pixmap, sizeof(PixmapT));
}
