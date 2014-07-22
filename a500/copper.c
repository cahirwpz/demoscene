#include "hardware.h"
#include "coplist.h"
#include "interrupts.h"

static CopListT *cp = NULL;

void Load() {
  cp = NewCopList(100);
}

void Kill() {
  DeleteCopList(cp);
}

static CopInsT *upperColor = NULL;

__interrupt_handler void IntLevel3Handler() {
  static ULONG frameNumber = 0;

  if (custom->intreqr & INTF_VERTB) {
    frameNumber++;

    if (upperColor)
      CopInsSet16(upperColor, ((frameNumber & 63) < 32) ? 0x00f : 0x0f0);
  }

  /*
   * Clear interrupt flags for this level to avoid infinite re-entering
   * interrupt handler.
   */
  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

void Main() {
  APTR OldIntLevel3;

  /* Hardware initialization. */
  OldIntLevel3 = InterruptVector->IntLevel3;
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_LEVEL3;

  CopInit(cp);
  upperColor = CopSetRGB(cp, 0, 0xfff);
  CopWait(cp, 312/2, 0);
  CopSetRGB(cp, 0, 0xf00);
  CopEnd(cp);
  CopListActivate(cp);

  WaitMouse();

  /* Hardware release. */
  custom->intena = INTF_LEVEL3;
  InterruptVector->IntLevel3 = OldIntLevel3;
}
