#include <copper.h>

CopInsT *CopWaitSafe(CopListT *list, u_short vp, u_short hp) {
  if (!(list->flags & CLF_VPOVF) && (vp >= 256)) {
    /* Wait for last waitable position to control when overflow occurs. */
    CopWaitEOL(list, 255);
    list->flags |= CLF_VPOVF;
  }

  return CopWait(list, vp, hp);
}
