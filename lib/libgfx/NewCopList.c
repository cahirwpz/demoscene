#include <copper.h>
#include <memory.h>

CopListT *NewCopList(u_short length) {
  CopListT *list = MemAlloc(sizeof(CopListT) + length * sizeof(CopInsT),
                            MEMF_CHIP|MEMF_CLEAR);

  list->length = length;
  CopInit(list);
  return list;
}
