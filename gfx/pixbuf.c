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

  pixbuf->width = width;
  pixbuf->height = height;
  pixbuf->baseColor = 0;
  pixbuf->colors = 256;
  pixbuf->data = NewTable(uint8_t, width * height);

  return pixbuf;
}

PixBufT *NewPixBufFromFile(const StrT fileName) {
  uint16_t *data = ReadFileSimple(fileName);

  if (data) {
    uint16_t width = data[0];
    uint16_t height = data[1];
    uint16_t colors = data[2];

    PixBufT *pixbuf = NewPixBuf(width, height);

    pixbuf->colors = colors;
    memcpy(pixbuf->data, &data[3], width * height);

    LOG("Image '%s' has size (%d,%d) and %d colors.",
        fileName, width, height, colors);

    MemUnref(data);

    return pixbuf;
  }

  return NULL;
}

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette) {
  if (palette->count != pixbuf->colors) {
    LOG("PixBuf color number doesn't match palette (%d != %ld).",
        pixbuf->colors, palette->count);
  } else {
    int color = palette->start - pixbuf->baseColor;
    int i;

    for (i = 0; i < pixbuf->width * pixbuf->height; i++)
      pixbuf->data[i] += color;

    pixbuf->baseColor = palette->start;
  }
}

void PutPixel(PixBufT *pixbuf asm("a0"),
              size_t x asm("d1"), size_t y asm("d2"), uint32_t c asm("d0"))
{
  ASSERT(x < pixbuf->width, "x out of bound");
  ASSERT(y < pixbuf->height, "y out of bound");
  pixbuf->data[x + pixbuf->width * y] = c;
}

uint32_t GetPixel(PixBufT *pixbuf asm("a0"),
                  size_t x asm("d1"), size_t y asm("d2"))
{
  ASSERT(x < pixbuf->width, "x out of bound");
  ASSERT(y < pixbuf->height, "y out of bound");
  return pixbuf->data[x + pixbuf->width * y];
}
