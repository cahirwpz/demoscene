#include <copper.h>

CopListT *CopListReset(CopListT *list) {
  list->curr = list->entry;
  list->overflow = 0;
  return list;
}
