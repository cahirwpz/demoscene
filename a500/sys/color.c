#include "color.h"
#include "common.h"

UBYTE colortab[4096];

void InitColorTab() {
  WORD i, j, k;
  UBYTE *ptr = colortab;

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      WORD l = j - i;
      LONG r;
      for (k = 0, r = 0; k < 16; k++, r += l)
        *ptr++ = (i + div16(r, 15)) << 4;
    }
  }
}

ADD2INIT(InitColorTab, 0);
