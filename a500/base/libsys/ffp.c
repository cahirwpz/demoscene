#include "common.h"
#include "ffp.h"

#define FFPMantisse_Mask 0xFFFFFF00 /* 24 bit for the mantisse */
#define FFPExponent_Mask 0x0000007F /*  7 bit for the exponent */
#define FFPSign_Mask     0x00000080

#define IEEESPMantisse_Mask 0x007FFFFF /* 23 bit for the mantisse */
#define IEEESPExponent_Mask 0x7F800000 /*  8 bit for the exponent */
#define IEEESPSign_Mask     0x80000000

typedef union {
  u_int i;
  float f;
} binflt_t;

float SPFieee(float num asm("d0")) {
  binflt_t src = { .f = num };
  binflt_t dst = { .i = 0 };

  if (src.i != 0) {
    u_int s = (src.i & IEEESPSign_Mask) >> 24;
    u_int e = ((src.i & IEEESPExponent_Mask) >> 23) - 62;
    u_int m = ((src.i & IEEESPMantisse_Mask) << 8) | 0x80000000;
    dst.i = s | e | m;
  }

  return dst.f;
}
