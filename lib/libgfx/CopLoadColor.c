#include <copper.h>

CopInsT *CopLoadColor(CopListT *list, short start, short end, short color) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;
  short n = end - start - 1;
  short reg = CSREG(color[start]);

  do {
    *ins++ = reg;
    *ins++ = color;
    reg += 2;
  } while (--n != -1);

  list->curr = (CopInsT *)ins;
  return ptr;
}
