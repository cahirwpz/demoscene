#include <memory.h>
#include <pixmap.h>

void DeletePixmap(PixmapT *pixmap) {
  if (pixmap) {
    MemFree(pixmap->pixels);
    MemFree(pixmap);
  }
}
