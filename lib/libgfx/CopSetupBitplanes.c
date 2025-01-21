#include <copper.h>

CopInsPairT *CopSetupBitplanes(CopListT *list, const BitmapT *bitmap,
                               u_short depth)
{
  CopInsPairT *bplptr = CopInsPtr(list);

  short modulo = 0;
  short n = depth - 1;

  if (bitmap->flags & BM_INTERLEAVED)
    modulo = (short)bitmap->bytesPerRow * n;

  {
    void *const *planes = bitmap->planes;
    int i = 0;

    do {
      CopMove32(list, bplpt[i++], *planes++);
    } while (--n != -1);
  }

  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);
  return bplptr;
}
