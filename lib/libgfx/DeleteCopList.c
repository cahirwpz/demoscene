#include <debug.h>
#include <copper.h>
#include <system/memory.h>

void DeleteCopList(CopListT *list) {
  int unused = list->length - (list->curr - list->entry);
  if (unused >= 100)
    Log("Unused copper list entries: %d.\n", unused);
  MemFree(list);
}
