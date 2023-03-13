#include <copper.h>

CopInsT *CopLoadColorArray(CopListT *list, const u_short *col, short ncols,
                           short first) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;
  short reg = CSREG(color[first]);
  short n = ncols - 1;

  do {
    *ins++ = reg;
    *ins++ = *col++;
    reg += 2;
  } while (--n != -1);

  list->curr = (CopInsT *)ins;
  return ptr;
}
