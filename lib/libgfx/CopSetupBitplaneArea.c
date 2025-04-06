#include <copper.h>

void CopSetupBitplaneArea(CopListT *list, u_short mode, u_short depth,
                          const BitmapT *bitmap, short x, short y __unused,
                          const Area2D *area)
{
  void *const *planes = bitmap->planes;
  int start;
  short modulo;
  short w;
  short i;

  if (area) {
    w = (area->w + 15) & ~15;
    /* Seems that bitplane fetcher has to be active for at least two words! */
    if (w < 32)
      w = 32;
    start = bitmap->bytesPerRow * area->y + ((area->x >> 3) & ~1);
    modulo = bitmap->bytesPerRow - ((w >> 3) & ~1);
    x -= (area->x & 15);
  } else {
    w = (bitmap->width + 15) & ~15;
    start = 0;
    modulo = 0;
  }

  for (i = 0; i < depth; i++)
    CopMove32(list, bplpt[i], *planes++ + start);

  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);

  CopSetupBitplaneFetch(list, mode, x, w);
}
