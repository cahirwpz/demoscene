#include <copper.h>

void CopEnd(CopListT *list) {
  u_int *ins = (u_int *)list->curr;

  *ins++ = 0xfffffffe;

  list->curr = (CopInsT *)ins;
}
