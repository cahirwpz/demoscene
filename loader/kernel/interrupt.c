#include <interrupt.h>

void WaitIRQ(u_short mask) {
  while (!(custom->intreqr & mask));
  ClearIRQ(mask);
}
