#include <color.h>
#include <custom.h>

void FadeBlack(const u_short *colors, short count, u_int start, short step) {
  volatile u_short *reg = &custom->color[start];
 
  if (step < 0)
    step = 0;
  if (step > 15)
    step = 15;

  while (--count >= 0) {
    short to = *colors++;

    short r = ((to >> 4) & 0xf0) | step;
    short g = (to & 0xf0) | step;
    short b = ((to << 4) & 0xf0) | step;

    r = colortab[r];
    g = colortab[g];
    b = colortab[b];

    *reg++ = (r << 4) | g | (b >> 4);
  }
}
