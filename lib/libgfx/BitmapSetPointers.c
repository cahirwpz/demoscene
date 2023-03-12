#include <bitmap.h>

void BitmapSetPointers(BitmapT *bitmap, void *planes) {
  int modulo =
    (bitmap->flags & BM_INTERLEAVED) ? bitmap->bytesPerRow : bitmap->bplSize;
  short depth = bitmap->depth;
  void **planePtr = bitmap->planes;

  do {
    *planePtr++ = planes;
    planes += modulo;
  } while (depth--);
}
