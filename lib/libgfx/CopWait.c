#include <copper.h>

CopInsT *CopWait(CopListT *list, u_short vp, u_short hp) {
  CopInsT *ptr = list->curr;
  u_char *bp = (u_char *)ptr;
  u_short *wp;

  *bp++ = vp;
  *bp++ = hp | 1; 
  wp = (u_short *)bp;
  *wp++ = 0xfffe;

  list->curr = (CopInsT *)wp;
  return ptr;
}
