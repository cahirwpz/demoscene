#ifndef __PIXMAP_H__
#define __PIXMAP_H__

#include "gfx.h"

typedef enum {
  PM_NONE, PM_DEPTH_1, PM_DEPTH_2, PM_DEPTH_4, PM_DEPTH_8, PM_DEPTH_16,
  _PM_GRAY = 8,
  _PM_CMAP = 16,
  _PM_RGB  = 32,
} PixmapTypeT;

#define PM_CMAP4  (_PM_CMAP|PM_DEPTH_4)
#define PM_CMAP8  (_PM_CMAP|PM_DEPTH_8)
#define PM_GRAY4  (_PM_GRAY|PM_DEPTH_4)
#define PM_GRAY8  (_PM_GRAY|PM_DEPTH_8)
#define PM_GRAY16 (_PM_GRAY|PM_DEPTH_16)
#define PM_RGB12  (_PM_RGB|PM_DEPTH_4)
#define PM_RGB24  (_PM_RGB|PM_DEPTH_8)

#define PM_DEPTH_MASK 7

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

__regargs void PixmapConvert(PixmapT *pixmap, PixmapTypeT type);

#endif
