#include <bitmap.h>

void InitSharedBitmap(BitmapT *bitmap, u_short width, u_short height,
                      u_short depth, BitmapT *donor)
{
  u_short bytesPerRow = ((width + 15) & ~15) / 8;

  bitmap->width = width;
  bitmap->height = height;
  bitmap->depth = depth;
  bitmap->bytesPerRow = bytesPerRow;
  bitmap->bplSize = bytesPerRow * height;
  bitmap->flags = donor->flags;

  BitmapSetPointers(bitmap, donor->planes[0]);
}
