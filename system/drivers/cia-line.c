#include <system/cia.h>

/* All TOD registers latch on a read of MSB event and remain latched until
 * after a read of LSB event. */
u_int ReadLineCounter(void) {
  int res = 0;
  res |= ciab->ciatodhi;
  res <<= 8;
  res |= ciab->ciatodmid;
  res <<= 8;
  res |= ciab->ciatodlow;
  return res;
}

/* TOD is automatically stopped whenever a write to the register occurs. The
 * clock will not start again until after a write to the LSB event register. */
void WriteLineCounter(u_int line) {
  ciab->ciatodhi = line >> 16;
  ciab->ciatodmid = line >> 8;
  ciab->ciatodlow = line;
}
