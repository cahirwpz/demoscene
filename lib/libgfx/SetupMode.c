#include <playfield.h>

void SetupMode(u_short mode, u_short depth) {
  custom->bplcon0 = BPLCON0_BPU(depth) | BPLCON0_COLOR | mode;
  custom->bplcon2 = BPLCON2_PF2P2 | BPLCON2_PF1P2 | BPLCON2_PF2PRI;
  custom->bplcon3 = 0;
}
