#include <copper.h>

void CopUpdateBitplanes(CopInsPairT *bplptr, const BitmapT *bitmap, short n) {
  void **planes = bitmap->planes;

  while (--n >= 0)
    CopInsSet32(bplptr++, *planes++);
}
