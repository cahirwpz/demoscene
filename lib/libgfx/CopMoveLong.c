#include <copper.h>

CopInsT *CopMoveLong(CopListT *list, u_short reg, void *data) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;

  reg &= 0x01fe;

  *ins++ = reg;
  *ins++ = (u_int)data >> 16;
  *ins++ = reg + 2;
  *ins++ = (u_int)data;

  list->curr = (CopInsT *)ins;
  return ptr;
}
