#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "gfx/pixbuf.h"

PixBufT *NewPixBuf(size_t width, size_t height) {
  PixBufT *pixbuf = NEW_S(PixBufT);

  pixbuf->width = width;
  pixbuf->height = height;
  pixbuf->baseColor = 0;
  pixbuf->colors = 256;
  pixbuf->data = NEW_A(uint8_t, width * height);

  return pixbuf;
}

PixBufT *NewPixBufFromFile(const char *fileName, uint32_t memFlags) {
  uint16_t *data = ReadFileSimple(fileName, memFlags);

  if (data) {
    uint16_t width = data[0];
    uint16_t height = data[1];
    uint16_t colors = data[2];

    PixBufT *pixbuf = NewPixBuf(width, height);

    pixbuf->colors = colors;
    memcpy(pixbuf->data, &data[3], width * height);

    LOG("Image '%s' has size (%d,%d) and %d colors.\n",
        fileName, width, height, colors);

    DELETE(data);

    return pixbuf;
  }

  return NULL;
}

void DeletePixBuf(PixBufT *pixbuf) {
  if (pixbuf) {
    DELETE(pixbuf->data);
    DELETE(pixbuf);
  }
}

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette) {
  if (palette->count != pixbuf->colors) {
    LOG("PixBuf color number doesn't match palette (%d != %d).\n",
        pixbuf->colors, palette->count);
  } else {
    int color = palette->start - pixbuf->baseColor;
    int i;

    for (i = 0; i < pixbuf->width * pixbuf->height; i++)
      pixbuf->data[i] += color;

    pixbuf->baseColor = palette->start;
  }
}
