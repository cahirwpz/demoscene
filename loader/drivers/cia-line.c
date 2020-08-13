#include <cia.h>

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
