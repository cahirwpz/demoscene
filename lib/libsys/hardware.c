#include "hardware.h"

volatile struct Custom* const custom = (void *)0xdff000;
volatile struct CIA* const ciaa = (void *)0xbfe001;
volatile struct CIA* const ciab = (void *)0xbfd000;

void WaitMouse() {
  while (ciaa->ciapra & CIAF_GAMEPORT0);
}

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

__regargs void WaitLine(u_int line) {
  u_int mask = 0x1ff00;
  u_int vpos;

  line <<= 8;
  line &= mask;

  do {
    vpos = *vposr() & mask;
  } while (vpos != line);
}

__regargs int ReadICR(volatile struct CIA *cia) {
  static int oldicr = 0;
  u_char newicr = cia->ciaicr;
  if (newicr)
    oldicr = newicr;
  return oldicr;
}

__regargs void WaitTimerA(volatile struct CIA *cia, u_short delay) {
  cia->ciacra |= CIACRAF_RUNMODE;
  cia->ciaicr = CIAICRF_TA;
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  while (!(cia->ciaicr & CIAICRF_TA));
}

__regargs void WaitTimerB(volatile struct CIA *cia, u_short delay) {
  cia->ciacra |= CIACRBF_RUNMODE;
  cia->ciaicr = CIAICRF_TB;
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  while (!(cia->ciaicr & CIAICRF_TB));
}

/* All TOD registers latch on a read of MSB event and remain latched until
 * after a read of LSB event. */

int ReadLineCounter() {
  int res = 0;
  res |= ciab->ciatodhi;
  res <<= 8;
  res |= ciab->ciatodmid;
  res <<= 8;
  res |= ciab->ciatodlow;
  return res;
}

int ReadFrameCounter() {
  int res = 0;
  res |= ciaa->ciatodhi;
  res <<= 8;
  res |= ciaa->ciatodmid;
  res <<= 8;
  res |= ciaa->ciatodlow;
  return res;
}

/* TOD is automatically stopped whenever a write to the register occurs. The
 * clock will not start again until after a write to the LSB event register. */

void SetFrameCounter(u_int frame) {
  ciaa->ciatodhi = frame >> 16;
  ciaa->ciatodmid = frame >> 8;
  ciaa->ciatodlow = frame;
}
