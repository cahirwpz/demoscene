#include <custom.h>
#include <palette.h>

void LoadColorArray(const u_short *colors, short count, int start) {
  short n = min((short)count, (short)(32 - start)) - 1;
  volatile u_short *colreg = custom->color + start;

  do {
    *colreg++ = *colors++;
  } while (--n != -1);
}
