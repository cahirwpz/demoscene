#include <copper.h>

CopInsT *CopLoadColor(CopListT *list, u_short start, u_short end, u_short color)
{
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;

  while (start <= end) {
    *ins++ = CSREG(color[start++]);
    *ins++ = color;
  }

  list->curr = (CopInsT *)ins;
  return ptr;
}
