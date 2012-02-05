#include "system/memory.h"
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

void DeletePixBuf(PixBufT *pixbuf) {
  if (pixbuf)
    DELETE(pixbuf->data);

  DELETE(pixbuf);
}
