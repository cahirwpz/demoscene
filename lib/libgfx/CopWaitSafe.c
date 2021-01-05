#include <copper.h>

CopInsT *CopWaitSafe(CopListT *list, short vp, short hp) {
  if (vp >= 256 && !list->overflow) {
    list->overflow = -1;
    /* Wait for last waitable position to control when overflow occurs. */
    *((u_int *)list->curr)++ = 0xffdffffe;
  }
  return CopWait(list, vp, hp);
}
