#include <string.h>

#include "system/memory.h"
#include "system/fileio.h"
#include "system/debug.h"
#include "gfx/pixbuf.h"

PixBufT *NewPixBuf(int width, int height) {
  PixBufT *pixbuf = NEW_S(PixBufT);

  if (pixbuf) {
    pixbuf->width = width;
    pixbuf->height = height;
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

  PixBufT *pixbuf = NewPixBuf(width, height);

  if (pixbuf)
    memcpy(pixbuf->data, &data[2], width * height);

  LOG("Image '%s' has size (%ld,%ld).\n",
      fileName, (ULONG)width, (ULONG)height);

  DELETE(data);

  return pixbuf;
}

void DeletePixBuf(PixBufT *pixbuf) {
  if (pixbuf)
    DELETE(pixbuf->data);

  DELETE(pixbuf);
}
