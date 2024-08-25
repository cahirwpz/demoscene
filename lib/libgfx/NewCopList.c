#include <copper.h>
#include <system/memory.h>

CopListT *NewCopList(int length) {
  CopListT *list =
    MemAlloc(sizeof(CopListT) + length * sizeof(CopInsT), MEMF_CHIP);
  list->length = length;
  list->curr = list->entry;
  list->overflow = 0;
  list->finished = 0;
  return list;
}
