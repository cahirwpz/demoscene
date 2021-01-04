#include <copper.h>

CopInsT *CopSkipMask(CopListT *list, u_short vp, u_short hp,
                     u_short vpmask asm("d2"), u_short hpmask asm("d3"))
{
  CopInsT *ptr = (CopInsT *)list->curr;
  u_char *ins = (u_char *)ptr;

  *ins++ = vp;
  *ins++ = hp | 1;
  *ins++ = 0x80 | vpmask;
  *ins++ = hpmask | 1;

  list->curr = (CopInsT *)ins;
  return ptr;
}
