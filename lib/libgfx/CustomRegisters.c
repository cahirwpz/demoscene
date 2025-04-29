#include <custom.h>

u_short SetBplcon3(u_short val, u_short mask) {
  /* default values for AGA compatibility */
  static u_short __bplcon3 = BPLCON3_PF2OF(3) | BPLCON3_SPRES(0);
  __bplcon3 = (__bplcon3 & ~mask) | (val & mask);
  custom->bplcon3 = __bplcon3;
  return __bplcon3;
}
