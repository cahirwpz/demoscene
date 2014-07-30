#ifndef __PIXMAP_H__
#define __PIXMAP_H__

#include "gfx.h"

typedef enum { PM_GRAY, PM_CMAP, PM_RGB4 } PixmapTypeT;

typedef struct Pixmap {
  PixmapTypeT type;
  UWORD width, height;
  PaletteT *palette;
  APTR pixels;
} PixmapT;

__regargs PixmapT *NewPixmap(UWORD width, UWORD height, PixmapTypeT type);
__regargs void DeletePixmap(PixmapT *pixmap);

#endif
