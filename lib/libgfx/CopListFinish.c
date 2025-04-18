#include <copper.h>
#include <debug.h>

CopListT *CopListFinish(CopListT *list) {
  CopInsT *ins = list->curr;
  stli(ins, 0xfffffffe);
  list->curr = ins;
  {
    ptrdiff_t used = (ptrdiff_t)(list->curr - list->entry);
    Log("[CopList] %p: used slots %ld/%d\n", list, used, list->length);
    if (used > list->length)
      Panic("[CopList] Overflow detected!");
  }
  list->finished = -1;
  return list;
}
