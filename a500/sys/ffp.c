#include "ffp.h"

#define FFPMantisse_Mask 0xFFFFFF00 /* 24 bit for the mantisse */
#define FFPExponent_Mask 0x0000007F /*  7 bit for the exponent */
#define FFPSign_Mask     0x00000080

#define IEEESPMantisse_Mask 0x007FFFFF /* 23 bit for the mantisse */
#define IEEESPExponent_Mask 0x7F800000 /*  8 bit for the exponent */
#define IEEESPSign_Mask     0x80000000

FLOAT SPFieee(FLOAT num asm("d0")) {
  ULONG ieee = *(ULONG *)&num;
  ULONG s, e, m, ffp = 0;

  if (ieee != 0) {
    s = (ieee & IEEESPSign_Mask) >> 24;
    e = ((ieee & IEEESPExponent_Mask) >> 23) - 62;
    m = ((ieee & IEEESPMantisse_Mask) << 8) | 0x80000000;
    ffp = s | e | m;
  }

  return *(FLOAT *)&ffp;
}
