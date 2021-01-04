#include <copper.h>

CopInsT *CopWaitMask(CopListT *list, u_short vp, u_short hp,
                     u_short vpmask asm("d2"), u_short hpmask asm("d3"))
{
  CopInsT *ptr = list->curr;
  u_char *ins = (u_char *)ptr;

  *ins++ = vp;
  *ins++ = hp | 1;
  *ins++ = 0x80 | vpmask;
  *ins++ = hpmask & 0xfe;

  list->curr = (CopInsT *)ins;
  return ptr;
}
