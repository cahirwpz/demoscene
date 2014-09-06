#include "ffp.h"

FLOAT SPFieee(FLOAT num) {
  ULONG ieee = *(ULONG *)&num;
  ULONG s = (ieee >> 31);
  ULONG e = (ieee >> 23) & 0xff;
  ULONG m = (ieee & 0x007fffff);
  ULONG ffp;

  m |= 0x800000;
  e -= 62;

  ffp = (s << 7) | (e & 0x7f) | (m << 8);

  return *(FLOAT *)&ffp;
}
