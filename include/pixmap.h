#ifndef __PIXMAP_H__
#define __PIXMAP_H__

#include "gfx.h"

typedef enum {
  PM_NONE, PM_DEPTH_4, PM_DEPTH_8,
  _PM_CMAP = 4,
  _PM_RGB  = 8,
} PixmapTypeT;

#define PM_CMAP4  (_PM_CMAP|PM_DEPTH_4)
#define PM_CMAP8  (_PM_CMAP|PM_DEPTH_8)
#define PM_RGB12  (_PM_RGB|PM_DEPTH_4)

#define PM_DEPTH_MASK 3

typedef struct Pixmap {
  PixmapTypeT type;
  short width, height;
  void *pixels;
} PixmapT;

static inline void InitSharedPixmap(PixmapT *pixmap, short width, short height,
                                    PixmapTypeT type, void *pixels) 
{
  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = pixels;
}

PixmapT *NewPixmap(short width, short height, 
                             PixmapTypeT type, u_int memFlags);
void DeletePixmap(PixmapT *pixmap);

/* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 b0 a2 b1 a1 b2 a3 b3] */
void PixmapScramble_4_1(const PixmapT *pixmap);
/* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 a1 b0 b1 a2 a3 b2 b3] */
void PixmapScramble_4_2(const PixmapT *pixmap);

#endif
