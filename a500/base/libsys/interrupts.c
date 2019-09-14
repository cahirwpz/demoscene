#include "interrupts.h"
#include "hardware.h"

void EnableINT(u_short mask) {
  custom->intena = INTF_SETCLR | mask; 
}

void DisableINT(u_short mask) {
  custom->intena = mask;
}

void ClearIRQ(u_short mask) {
  custom->intreq = mask;
}

void WaitIRQ(u_short mask) {
  while (!(custom->intreqr & mask));
  custom->intreq = mask;
}
