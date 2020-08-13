#include "hardware.h"

volatile struct Custom* const custom = (void *)0xdff000;

void EnableDMA(u_short mask) {
  custom->dmacon = DMAF_SETCLR | mask;
}

void DisableDMA(u_short mask) {
  custom->dmacon = mask;
}

static inline volatile u_int *vposr(void) {
  return (volatile u_int *)&custom->vposr;
}

int RasterLine() {
  return (*vposr() & 0x1ff00) >> 8;
}

void WaitLine(u_int line) {
  u_int mask = 0x1ff00;
  u_int vpos;

  line <<= 8;
  line &= mask;

  do {
    vpos = *vposr() & mask;
  } while (vpos != line);
}

