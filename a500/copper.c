#include "hardware.h"
#include "coplist.h"
#include "interrupts.h"

static CopInsT *upperColor = NULL;
static ULONG frameNumber = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    frameNumber++;

    if (upperColor)
      upperColor->move.data = ((frameNumber & 63) < 32) ? 0x00f : 0x0f0;
  }

  /*
   * Clear interrupt flags for this level to avoid infinite re-entering
   * interrupt handler.
   */
  custom->intreq = INTF_LEVEL3;
}

void Load() {}
void Kill() {}

void Main() {
  APTR OldIntLevel3;

  /* Hardware initialization. */
  {
    OldIntLevel3 = InterruptVector->IntLevel3;
    InterruptVector->IntLevel3 = IntLevel3Handler;
    custom->intena = INTF_SETCLR | INTF_LEVEL3 | INTF_INTEN;
  }

  {
    CopListT *cp = NewCopList(100);

    CopInit(cp);
    upperColor = CopMove16(cp, color[0], 0xfff);
    CopWait(cp, 312/2, 0);
    CopMove16(cp, color[0], 0xf00);
    CopEnd(cp);
    CopListActivate(cp);

    WaitMouse();

    upperColor = NULL;

    DeleteCopList(cp);
  }

  /* Hardware release. */
  {
    custom->intena = INTF_LEVEL3;
    InterruptVector->IntLevel3 = OldIntLevel3;
  }
}
