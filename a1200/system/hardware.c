#include "system/hardware.h"

volatile struct Custom* const custom = (APTR)0xdff000;
volatile struct CIA* const ciaa = (APTR)0xbfe001;
volatile struct CIA* const ciab = (APTR)0xbfd000;

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

void SetFrameCounter(int frame) {
  ciaa->ciatodhi = frame >> 16;
  ciaa->ciatodmid = frame >> 8;
  ciaa->ciatodlow = frame;
}
