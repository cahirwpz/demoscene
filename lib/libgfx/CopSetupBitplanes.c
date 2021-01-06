#include <copper.h>

void CopSetupBitplanes(CopListT *list, CopInsT **bplptr,
                       const BitmapT *bitmap, u_short depth) 
{
  {
    void **planes = bitmap->planes;
    short n = depth - 1;
    short reg = CSREG(bplpt);

    do {
      CopInsT *ins = CopMoveLong(list, reg, (int)(*planes++));

      if (bplptr)
        *bplptr++ = ins;

      reg += 4;
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
