#include <custom.h>
#include <palette.h>

void LoadPalette(const PaletteT *palette, u_int start) {
  u_short *col = palette->colors;
  short n = min((short)palette->count, (short)(32 - start)) - 1;
  volatile u_short *colreg = custom->color + start;

  do {
    *colreg++ = *col++;
  } while (--n != -1);
}
