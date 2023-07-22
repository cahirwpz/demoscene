#include <copper.h>

void CopListReset(CopListT *list) {
  list->curr = list->entry;
  list->overflow = 0;
}
