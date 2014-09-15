#ifndef __PIXMAP_H__
#define __PIXMAP_H__

#include "gfx.h"

typedef enum { PM_GRAY, PM_CMAP, PM_RGB4, PM_GRAY4 } PixmapTypeT;

typedef struct Pixmap {
  PixmapTypeT type;
  UWORD width, height;
  PaletteT *palette;
  APTR pixels;
} PixmapT;

static inline void InitSharedPixmap(PixmapT *pixmap, UWORD width, UWORD height,
                                    PixmapTypeT type, APTR pixels) 
{
  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->palette = NULL;
  pixmap->pixels = pixels;
}

__regargs PixmapT *NewPixmap(UWORD width, UWORD height, 
                             PixmapTypeT type, ULONG memoryAttributes);
__regargs PixmapT *ClonePixmap(PixmapT *pixmap);
__regargs void DeletePixmap(PixmapT *pixmap);

#endif
