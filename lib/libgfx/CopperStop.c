#include <custom.h>
#include <copper.h>

void CopperStop(void) {
  /* Load empty list and instruct copper to jump to them immediately. */
  custom->cop2lc = (u_int)&NullCopperList;
  custom->copjmp2 = 0;
  custom->cop1lc = (u_int)&NullCopperList;
  custom->copjmp1 = 0;
  /* Disable copper DMA and all systems that rely on frame-by-frame refresh
   * by Copper. */
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_SPRITE);
}
