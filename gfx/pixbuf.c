#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "gfx/pixbuf.h"

static void DeletePixBuf(PixBufT *pixbuf) {
  MemUnref(pixbuf->data);
}

PixBufT *NewPixBuf(size_t width, size_t height) {
  PixBufT *pixbuf = NewRecordGC(PixBufT, (FreeFuncT)DeletePixBuf);

  pixbuf->type = PIXBUF_CLUT;
  pixbuf->width = width;
  pixbuf->height = height;
  pixbuf->baseColor = 0;
  pixbuf->colors = 256;
  pixbuf->data = NewTable(uint8_t, width * height);

  return pixbuf;
}

typedef struct DiskPixBuf {
  uint16_t type;
  uint16_t width;
  uint16_t height;
  uint32_t colors;
  uint8_t  data[0];
} DiskPixBufT;

PixBufT *NewPixBufFromFile(const StrT fileName) {
  DiskPixBufT *file = (DiskPixBufT *)ReadFileSimple(fileName);

  if (file) {
    PixBufT *pixbuf = NewPixBuf(file->width, file->height);

    pixbuf->type = file->type;
    pixbuf->colors = file->colors;

    memcpy(pixbuf->data, file->data, file->width * file->height);

    LOG("Image '%s' has size (%d,%d) and %d colors.",
        fileName, (int)file->width, (int)file->height, (int)file->colors);

    MemUnref(file);

    return pixbuf;
  }

  return NULL;
}

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette) {
  ASSERT(pixbuf->type == PIXBUF_CLUT, "Cannot remap non-CLUT image!");

  if (palette->count != pixbuf->colors) {
    LOG("PixBuf color number doesn't match palette (%d != %d).",
        (int)pixbuf->colors, (int)palette->count);
  } else {
    int color = palette->start - pixbuf->baseColor;
    int i;

    for (i = 0; i < pixbuf->width * pixbuf->height; i++)
      pixbuf->data[i] += color;

    pixbuf->baseColor = palette->start;
  }
}

static const size_t inline GetPixelIndex(PixBufT *pixbuf, ssize_t x, ssize_t y) {
  ASSERT((x >= 0) && (x < pixbuf->width), "x (%d) out of bound!", x);
  ASSERT((y >= 0) && (y < pixbuf->height), "y (%d) out of bound!", y);
  return x + pixbuf->width * y;
}

void PutPixel(PixBufT *pixbuf asm("a0"),
              ssize_t x asm("a1"), ssize_t y asm("d1"), uint8_t c asm("d0"))
{
  size_t index = GetPixelIndex(pixbuf, x, y);
  pixbuf->data[index] = c;
}

uint8_t GetPixel(PixBufT *pixbuf asm("a0"),
                 ssize_t x asm("d0"), ssize_t y asm("d1"))
{
  return pixbuf->data[GetPixelIndex(pixbuf, x, y)];
}

void PutPixelRGB(PixBufT *pixbuf asm("a0"),
                 ssize_t x asm("a1"), ssize_t y asm("d1"), ColorT c asm("d0"))
{
  size_t index = GetPixelIndex(pixbuf, x, y);
  ((uint32_t *)pixbuf->data)[index] = *(uint32_t *)&c;
}

ColorT GetPixelRGB(PixBufT *pixbuf asm("a0"),
                   ssize_t x asm("d0"), ssize_t y asm("d1"))
{
  return *(ColorT *)&pixbuf->data[GetPixelIndex(pixbuf, x, y)];
}
