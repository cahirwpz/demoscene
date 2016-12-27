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

__regargs LONG ReadICR(volatile struct CIA *cia) {
  static LONG oldicr = 0;
  UBYTE newicr = cia->ciaicr;
  if (newicr)
    oldicr = newicr;
  return oldicr;
}

__regargs void WaitTimerA(volatile struct CIA *cia, UWORD delay) {
  cia->ciacra |= CIACRAF_RUNMODE;
  cia->ciaicr = CIAICRF_TA;
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  while (cia->ciaicr & CIAICRF_TA);
}

__regargs void WaitTimerB(volatile struct CIA *cia, UWORD delay) {
  cia->ciacra |= CIACRBF_RUNMODE;
  cia->ciaicr = CIAICRF_TB;
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  while (cia->ciaicr & CIAICRF_TB);
}

/* All TOD registers latch on a read of MSB event and remain latched until
 * after a read of LSB event. */

LONG ReadLineCounter() {
  LONG res = 0;
  res |= ciab->ciatodhi;
  res <<= 8;
  res |= ciab->ciatodmid;
  res <<= 8;
  res |= ciab->ciatodlow;
  return res;
}

LONG ReadFrameCounter() {
  LONG res = 0;
  res |= ciaa->ciatodhi;
  res <<= 8;
  res |= ciaa->ciatodmid;
  res <<= 8;
  res |= ciaa->ciatodlow;
  return res;
}

/* TOD is automatically stopped whenever a write to the register occurs. The
 * clock will not start again until after a write to the LSB event register. */

void SetFrameCounter(ULONG frame) {
  ciaa->ciatodhi = frame >> 16;
  ciaa->ciatodmid = frame >> 8;
  ciaa->ciatodlow = frame;
}
