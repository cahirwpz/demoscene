#ifndef __PIXMAP_H__
#define __PIXMAP_H__

#include "gfx.h"

typedef enum { PM_NONE, PM_GRAY, PM_CMAP, PM_RGB4, PM_GRAY4, PM_CMAP4 } PixmapTypeT;

typedef struct Pixmap {
  PixmapTypeT type;
  WORD width, height;
  PaletteT *palette;
  APTR pixels;
} PixmapT;

static inline void InitSharedPixmap(PixmapT *pixmap, WORD width, WORD height,
                                    PixmapTypeT type, APTR pixels) 
{
  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->palette = NULL;
  pixmap->pixels = pixels;
}

__regargs LONG PixmapSize(PixmapT *pixmap);
__regargs PixmapT *NewPixmap(WORD width, WORD height, 
                             PixmapTypeT type, ULONG memFlags);
__regargs PixmapT *ClonePixmap(PixmapT *pixmap);
__regargs void DeletePixmap(PixmapT *pixmap);

/* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 b0 a2 b1 a1 b2 a3 b3] */
__regargs void PixmapScramble_4_1(PixmapT *pixmap);
/* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 a1 b0 b1 a2 a3 b2 b3] */
__regargs void PixmapScramble_4_2(PixmapT *pixmap);

__regargs void PixmapExpandPixels(PixmapT *pixmap);

#endif
