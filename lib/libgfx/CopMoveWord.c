#include <copper.h>

CopInsT *CopMoveWord(CopListT *list, u_short reg, u_short data) {
  CopInsT *ptr = list->curr;
  u_short *ins = (u_short *)ptr;

  *ins++ = reg & 0x01fe;
  *ins++ = data;

  list->curr = (CopInsT *)ins;
  return ptr;
}
