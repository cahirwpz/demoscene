#include <custom.h>
#include <copper.h>

void CopperStop(void) {
  /* Disable copper DMA and all systems that rely on frame-by-frame refresh
   * by Copper. */
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_SPRITE);
}
