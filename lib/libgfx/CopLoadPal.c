#include <copper.h>

CopInsT *CopLoadPal(CopListT *list, const PaletteT *palette, short start) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;
  u_short *c = palette->colors;
  short n = min((short)palette->count, (short)(32 - start)) - 1;
  short reg = CSREG(color[start]);

  do {
    *ins++ = reg;
    *ins++ = *c++;
    reg += 2;
  } while (--n != -1);

  list->curr = (CopInsT *)ins;
  return ptr;
}
