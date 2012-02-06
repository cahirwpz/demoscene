#include <string.h>

#include "system/memory.h"
#include "system/fileio.h"
#include "system/debug.h"
#include "gfx/pixbuf.h"

PixBufT *NewPixBuf(size_t width, size_t height) {
  PixBufT *pixbuf = NEW_S(PixBufT);

  if (pixbuf) {
    pixbuf->width = width;
    pixbuf->height = height;
    pixbuf->baseColor = 0;
    pixbuf->colors = 256;
    pixbuf->data = NEW_A(uint8_t, width * height);

    if (pixbuf->data)
      return pixbuf;

    DeletePixBuf(pixbuf);
  }

  return NULL;
}

PixBufT *NewPixBufFromFile(const char *fileName, uint32_t memFlags) {
  uint16_t *data = ReadFileSimple(fileName, memFlags);

  if (!data)
    return NULL;

  uint16_t width = data[0];
  uint16_t height = data[1];
  uint16_t colors = data[2];

  PixBufT *pixbuf = NewPixBuf(width, height);

  if (pixbuf) {
    pixbuf->colors = colors;
    memcpy(pixbuf->data, &data[3], width * height);
  }

  LOG("Image '%s' has size (%ld,%ld) and %ld colors.\n",
      fileName, (ULONG)width, (ULONG)height, (ULONG)colors);

  DELETE(data);

  return pixbuf;
}

void DeletePixBuf(PixBufT *pixbuf) {
  if (pixbuf)
    DELETE(pixbuf->data);

  DELETE(pixbuf);
}

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette) {
  if (palette->count != pixbuf->colors) {
    LOG("PixBuf color number doesn't match palette (%ld != %ld).\n",
        (ULONG)pixbuf->colors, (ULONG)palette->count);
  } else {
    int color = palette->start - pixbuf->baseColor;
    int i;

    for (i = 0; i < pixbuf->width * pixbuf->height; i++)
      pixbuf->data[i] += color;
  }
}
