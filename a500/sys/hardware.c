#include "hardware.h"

volatile struct Custom* const custom = (APTR)0xdff000;
volatile struct CIA* const ciaa = (APTR)0xbfe001;
volatile struct CIA* const ciab = (APTR)0xbfd000;

void WaitMouse() {
  while (ciaa->ciapra & CIAF_GAMEPORT0);
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

/* Wait for a period of time shorter than one frame.
 * This is based upon a fact that VPOS register resolution is 280ns. */
__regargs void Wait280ns(ULONG delay) {
  volatile LONG *vposr = (volatile LONG *)&custom->vposr;
  LONG start = (*vposr) & 0x1ffff;
  LONG diff;

  do {
    diff = ((*vposr) & 0x1ffff) - start;
    if (diff < 0)
      diff += 0x20000;
  } while (diff < delay);
}

LONG ReadLineCounter() {
  return (ciab->ciatodlow | (ciab->ciatodmid << 8) | (ciab->ciatodhi << 16));
}

LONG ReadFrameCounter() {
  return (ciaa->ciatodlow | (ciaa->ciatodmid << 8) | (ciaa->ciatodhi << 16));
}
