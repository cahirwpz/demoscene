#include "color.h"
#include "common.h"

UBYTE colortab[4096];

void InitColorTab() {
  WORD i, j, k;
  UBYTE *ptr = colortab;

  Log("[Init] Color table.\n");

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      WORD l = j - i;
      LONG r;
      for (k = 0, r = 0; k < 16; k++, r += l)
        *ptr++ = (i + div16(r, 15)) << 4;
    }
  }
}

/* Each argument must be in range 0-15. */
__regargs UWORD ColorTransition(UWORD from, UWORD to, UWORD step) {
  WORD r = (from & 0xf00) | ((to >> 4) & 0x0f0) | step;
  WORD g = ((from << 4) & 0xf00) | (to & 0x0f0) | step;
  WORD b = ((from << 8) & 0xf00) | ((to << 4) & 0x0f0) | step;
  
  return (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
}

UWORD ColorTransitionRGB(WORD sr, WORD sg, WORD sb, WORD dr, WORD dg, WORD db, UWORD step) {
  WORD r = ((sr & 0xf0) << 4) | (dr & 0xf0) | step;
  WORD g = ((sg & 0xf0) << 4) | (dg & 0xf0) | step;
  WORD b = ((sb & 0xf0) << 4) | (db & 0xf0) | step;
  
  return (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
}

__regargs UWORD ColorIncreaseContrastRGB(WORD r, WORD g, WORD b, UWORD step) {
  r &= 0xf0;
  r += colortab[(WORD)(r | step)];
  if (r > 0xf0) r = 0xf0;

  g &= 0xf0;
  g += colortab[(WORD)(g | step)];
  if (g > 0xf0) g = 0xf0;

  b &= 0xf0;
  b += colortab[(WORD)(b | step)];
  if (b > 0xf0) b = 0xf0;

  return (r << 4) | g | (b >> 4);
}

__regargs UWORD ColorDecreaseContrastRGB(WORD r, WORD g, WORD b, UWORD step) {
  r &= 0xf0;
  r -= colortab[(WORD)(r | step)];
  if (r < 0) r = 0;

  g &= 0xf0;
  g -= colortab[(WORD)(g | step)];
  if (g < 0) g = 0;

  b &= 0xf0;
  b -= colortab[(WORD)(b | step)];
  if (b < 0) b = 0;

  return (r << 4) | g | (b >> 4);
}

ADD2INIT(InitColorTab, 0);
