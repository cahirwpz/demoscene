#include <system/cia.h>
#include <system/task.h>
#include <system/cpu.h>

/* All TOD registers latch on a read of MSB event and remain latched
 * until after a read of LSB event. */

u_int ReadFrameCounter(void) {
  u_int res = 0;
  u_short ipl = SetIPL(SR_IM);
  res |= ciaa->ciatodhi;
  res <<= 8;
  res |= ciaa->ciatodmid;
  res <<= 8;
  res |= ciaa->ciatodlow;
  (void)SetIPL(ipl);
  return res;
}

/* TOD is automatically stopped whenever a write to the register occurs. The
 * clock will not start again until after a write to the LSB event register. */

void SetFrameCounter(u_int frame) {
  u_short ipl = SetIPL(SR_IM);
  ciaa->ciatodhi = frame >> 16;
  ciaa->ciatodmid = frame >> 8;
  ciaa->ciatodlow = frame;
  (void)SetIPL(ipl);
}
