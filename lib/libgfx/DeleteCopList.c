#include <debug.h>
#include <copper.h>
#include <system/memory.h>

void DeleteCopList(CopListT *list) {
  MemFree(list);
}
