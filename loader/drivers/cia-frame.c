#include <cia.h>
#include <interrupt.h>

/* All TOD registers latch on a read of MSB event and remain latched
 * until after a read of LSB event. */

u_int ReadFrameCounter(void) {
  u_int res = 0;
  IntrDisable();
  res |= ciaa->ciatodhi;
  res <<= 8;
  res |= ciaa->ciatodmid;
  res <<= 8;
  res |= ciaa->ciatodlow;
  IntrEnable();
  return res;
}

/* TOD is automatically stopped whenever a write to the register occurs. The
 * clock will not start again until after a write to the LSB event register. */

void SetFrameCounter(u_int frame) {
  IntrDisable();
  ciaa->ciatodhi = frame >> 16;
  ciaa->ciatodmid = frame >> 8;
  ciaa->ciatodlow = frame;
  IntrEnable();
}
