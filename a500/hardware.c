#include "hardware.h"

volatile struct Custom* const custom = (APTR)0xdff000;
volatile struct CIA* const ciaa = (APTR)0xbfe001;
volatile struct CIA* const ciab = (APTR)0xbfd000;

void WaitMouse() {
  while (ciaa->ciapra & CIAF_GAMEPORT0);
}

void WaitBlitter() {
  while (custom->dmaconr & DMAF_BLTDONE);
}

__regargs void WaitLine(ULONG line) {
  ULONG mask = 0x1ff00;
  ULONG vpos;

  line <<= 8;
  line &= mask;

  do {
    vpos = (*(volatile ULONG *)&custom->vposr) & mask;
  } while (vpos != line);
}
