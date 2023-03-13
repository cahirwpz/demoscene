#include <copper.h>

CopInsT *CopLoadPal(CopListT *list, const PaletteT *pal, short first) {
  short n = min((short)pal->count, (short)(32 - first));
  return CopLoadColorArray(list, pal->colors, n, first);
}
