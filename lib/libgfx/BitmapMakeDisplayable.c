#include <bitmap.h>
#include <string.h>
#include <system/memory.h>

void BitmapMakeDisplayable(BitmapT *bitmap) {
  if (!(bitmap->flags & BM_CPUONLY)) {
    u_int size = BitmapSize(bitmap);
    void *planes = MemAlloc(size, MEMF_CHIP);

    memcpy(planes, bitmap->planes[0], size - BM_EXTRA);
    MemFree(bitmap->planes[0]);

    bitmap->flags ^= BM_CPUONLY;
    BitmapSetPointers(bitmap, planes);
  }
}
