#include <color.h>
#include <common.h>
#include <debug.h>
#include <linkerset.h>

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

ADD2INIT(InitColorTab, 0);
