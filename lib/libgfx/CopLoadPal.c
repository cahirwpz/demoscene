#include <copper.h>

CopInsT *CopLoadPal(CopListT *list, const PaletteT *palette, u_short start) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;
  u_short *c = palette->colors;
  short n = min(palette->count, (u_short)(32 - start)) - 1;

  do {
    *ins++ = CSREG(color[start++]);
    *ins++ = *c++;
  } while (--n != -1);

  list->curr = (CopInsT *)ins;
  return ptr;
}
