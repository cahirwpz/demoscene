#include <blitter.h>
#include <system/interrupt.h>

void BlitterStop(void) {
  DisableINT(INTF_BLIT);
  WaitBlitter();
  DisableDMA(DMAF_BLITTER | DMAF_BLITHOG);
}
