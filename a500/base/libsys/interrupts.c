#include "interrupts.h"

void EnableINT(UWORD mask) {
  custom->intena = INTF_SETCLR | mask; 
}

void DisableINT(UWORD mask) {
  custom->intena = mask;
}

void ClearIRQ(UWORD mask) {
  custom->intreq = mask;
}

void WaitIRQ(UWORD mask) {
  while (!(custom->intreqr & mask));
  custom->intreq = mask;
}
