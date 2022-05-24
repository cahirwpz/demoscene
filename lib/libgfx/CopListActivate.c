#include <copper.h>

void CopListActivate(CopListT *list) {
  /* Enable copper DMA */
  EnableDMA(DMAF_MASTER | DMAF_COPPER);
  /* Write copper list address. */
  custom->cop1lc = (u_int)list->entry;
  /* Wait for vertical blank to make sure the list is active. */
  WaitVBlank();
}
