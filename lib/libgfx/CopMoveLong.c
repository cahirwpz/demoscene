#include <copper.h>

CopInsT *CopMoveLong(CopListT *list, u_short reg, void *ptr) {
  CopInsT *ins = list->curr;
  u_int data = (u_int)ptr;

  reg &= 0x01fe;
  reg += 2;

  *((u_short *)ins)++ = reg;
  *((u_short *)ins)++ = data;
  *((u_short *)ins)++ = reg - 2;
  *((u_short *)ins)++ = swap16(data);

  list->curr = ins;
  return ins - 2;
}
