#include <debug.h>
#include <copper.h>

void CopListActivate(CopListT *list) {
  if ((list->curr - list->entry) > list->length)
    PANIC();
  if (list->finished == 0)
    PANIC();
  /* Write copper list address. */
  custom->cop1lc = (u_int)list->entry;
  /* Enable copper DMA */
  EnableDMA(DMAF_MASTER | DMAF_COPPER);
  /* Wait for vertical blank to make sure the list is active. */
  WaitVBlank();
}
