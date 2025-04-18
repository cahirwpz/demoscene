#include <copper.h>

CopInsT *CopLoadColorArray(CopListT *list, const u_short *colors, short count,
                           int start) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;
  short reg = CSREG(color[start]);
  short n = min((short)count, (short)(32 - start)) - 1;

  do {
    *ins++ = reg;
    *ins++ = *colors++;
    reg += 2;
  } while (--n != -1);

  list->curr = (CopInsT *)ins;
  return ptr;
}
