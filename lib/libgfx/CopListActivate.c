#include <debug.h>
#include <copper.h>

void CopListActivate(CopListT *list) {
  if (list->finished == 0)
    Panic("[CopList] Cannot activate list that is not finished!");
  /* Write copper list address. */
  custom->cop1lc = (u_int)list->entry;
  /* Enable copper DMA */
  EnableDMA(DMAF_MASTER | DMAF_COPPER);
  /* Wait for vertical blank to make sure the list is active. */
  WaitVBlank();
}
