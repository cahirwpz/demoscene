#include <proto/exec.h>

#include "memory.h"
#include "pixmap.h"

static __regargs WORD BytesPerPixel(PixmapTypeT type, WORD width) {
  if (type == PM_RGB4)
    width *= 2;
  else if (type == PM_GRAY4 || type == PM_CMAP4)
    width /= 2;
  return width;
}

__regargs PixmapT *NewPixmap(WORD width, WORD height, 
                             PixmapTypeT type, ULONG memoryAttributes)
{
  PixmapT *pixmap = MemAlloc(sizeof(PixmapT), MEMF_PUBLIC|MEMF_CLEAR);
  LONG size = BytesPerPixel(type, width) * height;

  pixmap->type = type;
  pixmap->width = width;
  pixmap->height = height;
  pixmap->pixels = MemAlloc(size, memoryAttributes);

  return pixmap;
}

__regargs PixmapT *ClonePixmap(PixmapT *pixmap) {
  PixmapT *clone = NewPixmap(pixmap->width, pixmap->height,
                             pixmap->type, TypeOfMem(pixmap->pixels));
  LONG size = BytesPerPixel(pixmap->type, pixmap->width) * pixmap->height;

  memcpy(clone->pixels, pixmap->pixels, size);

  return clone;
}

__regargs void DeletePixmap(PixmapT *pixmap) {
  LONG size = BytesPerPixel(pixmap->type, pixmap->width) * pixmap->height;

  MemFree(pixmap->pixels, size);
  MemFree(pixmap, sizeof(PixmapT));
}

__regargs void PixmapScramble_4_1(PixmapT *pixmap) {
  if (pixmap->type == PM_GRAY4 || pixmap->type == PM_CMAP4) {
    ULONG *data = pixmap->pixels;
    WORD n = pixmap->width * pixmap->height / 8;
    register ULONG m0 asm("d6") = 0xa5a5a5a5;
    register ULONG m1 asm("d7") = 0x0a0a0a0a;

    /* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 b0 a2 b1 a1 b2 a3 b3] */
    while (--n >= 0) {
      ULONG c = *data;
      *data++ = (c & m0) | ((c >> 3) & m1) | ((c & m1) << 3);
    }
  }
}

__regargs void PixmapScramble_4_2(PixmapT *pixmap) {
  if (pixmap->type == PM_GRAY4 || pixmap->type == PM_CMAP4) {
    ULONG *data = pixmap->pixels;
    WORD n = pixmap->width * pixmap->height / 8;
    register ULONG m0 asm("d6") = 0xc3c3c3c3;
    register ULONG m1 asm("d7") = 0x0c0c0c0c;

    /* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 a1 b0 b1 a2 a3 b2 b3] */
    while (--n >= 0) {
      ULONG c = *data;
      *data++ = (c & m0) | ((c >> 2) & m1) | ((c & m1) << 2);
    }
  }
}
