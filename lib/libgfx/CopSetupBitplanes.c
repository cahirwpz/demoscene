#include <copper.h>

void CopSetupBitplanes(CopListT *list, CopInsT **bplptr,
                       const BitmapT *bitmap, u_short depth) 
{
  {
    void **planes = bitmap->planes;
    short n = depth - 1;
    short i = 0;

    do {
      CopInsT *ins = CopMove32(list, bplpt[i++], *planes++);

      if (bplptr)
        *bplptr++ = ins;
    } while (--n != -1);
  }

  {
    short modulo = 0;

    if (bitmap->flags & BM_INTERLEAVED)
      modulo = (short)bitmap->bytesPerRow * (short)(depth - 1);

    CopMove16(list, bpl1mod, modulo);
    CopMove16(list, bpl2mod, modulo);
  }
}
