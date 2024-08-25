#include <copper.h>
#include <debug.h>

CopListT *CopListFinish(CopListT *list) {
  CopInsT *ins = list->curr;
  *((u_int *)ins)++ = 0xfffffffe;
  list->curr = ins;
  {
    ptrdiff_t used = (ptrdiff_t)(list->curr - list->entry);
    Log("Used copper list %p slots: %ld/%d\n", list, used, list->length);
    if (used > list->length)
      PANIC();
  }
  list->finished = -1;
  return list;
}
