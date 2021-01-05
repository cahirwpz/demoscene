#include <copper.h>

CopInsT *CopMoveWord(CopListT *list, u_short reg, short data) {
  CopInsT *ins = list->curr;

  *((u_short *)ins)++ = reg & 0x01fe;
  *((u_short *)ins)++ = data;

  list->curr = ins;
  return ins - 1;
}
