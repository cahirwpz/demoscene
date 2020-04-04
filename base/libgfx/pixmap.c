#include "memory.h"
#include "pixmap.h"

static short bitdepth[] = { 0, 1, 2, 4, 8, 16 };

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

__regargs PixmapT *NewPixmap(short width, short height, 
                             PixmapTypeT type, u_int memoryAttributes)
{
  PixmapT *pixmap = MemAlloc(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = MemAlloc(PixmapSize(pixmap), memoryAttributes);

  return pixmap;
}

__regargs void DeletePixmap(PixmapT *pixmap) {
  if (pixmap) {
    MemFree(pixmap->pixels);
    MemFree(pixmap);
  }
}

__regargs void PixmapScramble_4_1(PixmapT *pixmap) {
  if (pixmap->type == PM_CMAP4) {
    u_int *data = pixmap->pixels;
    short n = pixmap->width * pixmap->height / 8;
    register u_int m0 asm("d6") = 0xa5a5a5a5;
    register u_int m1 asm("d7") = 0x0a0a0a0a;

    /* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 b0 a2 b1 a1 b2 a3 b3] */
    while (--n >= 0) {
      u_int c = *data;
      *data++ = (c & m0) | ((c >> 3) & m1) | ((c & m1) << 3);
    }
  }
}

__regargs void PixmapScramble_4_2(PixmapT *pixmap) {
  if (pixmap->type == PM_CMAP4) {
    u_int *data = pixmap->pixels;
    short n = pixmap->width * pixmap->height / 8;
    register u_int m0 asm("d6") = 0xc3c3c3c3;
    register u_int m1 asm("d7") = 0x0c0c0c0c;

    /* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 a1 b0 b1 a2 a3 b2 b3] */
    while (--n >= 0) {
      u_int c = *data;
      *data++ = (c & m0) | ((c >> 2) & m1) | ((c & m1) << 2);
    }
  }
}
