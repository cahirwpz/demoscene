#include <copper.h>

void CopSetupBitplanesUpsideDown(CopListT *list, CopInsT **bplptr,
                                 const BitmapT *bitmap, u_short depth) 
{
  short bytesPerLine = bitmap->bytesPerRow;
  int start;

  if (bitmap->flags & BM_INTERLEAVED)
    bytesPerLine *= (short)bitmap->depth;

  start = bytesPerLine * (short)(bitmap->height - 1);

  {
    void **planes = bitmap->planes;
    short n = depth - 1;
    short reg = CSREG(bplpt);

    do {
      CopInsT *ins = CopMoveLong(list, reg, (int)(*planes++) + start);

      if (bplptr)
        *bplptr++ = ins;

      reg += 4;
    } while (--n != -1);
  }

  {
    short modulo;

    if (bitmap->flags & BM_INTERLEAVED)
      modulo = (short)bitmap->bytesPerRow * (short)(depth + 1);
    else
      modulo = 2 * bitmap->bytesPerRow;

    CopMove16(list, bpl1mod, -modulo);
    CopMove16(list, bpl2mod, -modulo);
  }
}
