#include <interrupt.h>
#include <debug.h>

static short IntrNesting = 0;

void IntrEnable(void) {
  Assert(IntrNesting > 0);
  IntrNesting--;
}

void IntrDisable(void) {
  IntrNesting++;
}

void WaitIRQ(u_short mask) {
  while (!(custom->intreqr & mask));
  ClearIRQ(mask);
}

/* Predefined interrupt chains. */
INTCHAIN(PortsChain);
INTCHAIN(VertBlankChain);
INTCHAIN(ExterChain);

void AddIntServer(IntChainT *ic, IntServerT *is) {
  (void)ic;
  (void)is;
}

void RemIntServer(IntChainT *ic, IntServerT *is) {
  (void)ic;
  (void)is;
}
