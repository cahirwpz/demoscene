#include "color.h"
#include "common.h"

u_char colortab[4096];

void InitColorTab(void) {
  short i, j, k;
  u_char *ptr = colortab;

  Log("[Init] Color table.\n");

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      short l = j - i;
      int r;
      for (k = 0, r = 0; k < 16; k++, r += l)
        *ptr++ = (i + div16(r, 15)) << 4;
    }
  }
}

/* Each argument must be in range 0-15. */
__regargs u_short ColorTransition(u_short from, u_short to, u_short step) {
  short r = (from & 0xf00) | ((to >> 4) & 0x0f0) | step;
  short g = ((from << 4) & 0xf00) | (to & 0x0f0) | step;
  short b = ((from << 8) & 0xf00) | ((to << 4) & 0x0f0) | step;
  
  return (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
}

ADD2INIT(InitColorTab, 0);
