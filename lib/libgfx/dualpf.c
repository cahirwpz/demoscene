#include "copper.h"

__regargs void CopSetupDualPlayfield(CopListT *list, CopInsT **bplptr,
                                     const BitmapT *pf1, const BitmapT *pf2)
{
  void **planes1 = pf1->planes;
  void **planes2 = pf2->planes;
  short n = pf1->depth + pf2->depth - 1;
  short i = 0;

  do {
    void *plane = i & 1 ? *planes1++ : *planes2++;
    CopInsT *ins = CopMove32(list, bplpt[i++], plane);

    if (bplptr)
      *bplptr++ = ins;
  } while (--n != -1);


  {
    short bpl1mod = (short)pf1->bytesPerRow * (short)(pf1->depth - 1);
    short bpl2mod = (short)pf2->bytesPerRow * (short)(pf2->depth - 1);

    CopMove16(list, bpl1mod, pf1->flags & BM_INTERLEAVED ? bpl1mod : 0);
    CopMove16(list, bpl2mod, pf2->flags & BM_INTERLEAVED ? bpl2mod : 0);
  }
}
