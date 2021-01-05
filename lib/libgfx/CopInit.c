#include <copper.h>

void CopInit(CopListT *list) {
  list->curr = list->entry;
  list->overflow = 0;
}
