#include <debug.h>
#include <copper.h>
#include <system/memory.h>

void DeleteCopList(CopListT *list) {
  if (list->finished && *(u_int *)list->curr == 0xfffffffe)
    Panic("[CopList] End instruction was damaged!");
  MemFree(list);
}
