#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>

/* TODO: Change implementation to use software interrupts and timer.device */

__interrupt static int VBlankServer(int *counter asm("a1")) {
  (*counter)++;
  return 0;
}

static int VBlankCounter = 0;

static struct Interrupt VBlankInt = {
  {
    NULL,
    NULL,
    NT_INTERRUPT,
    -60,
    "VBlankCounter"
  },
  (APTR)&VBlankCounter,
  (APTR)VBlankServer
};

void InstallVBlankIntServer() {
  AddIntServer(INTB_VERTB, &VBlankInt);
}

void RemoveVBlankIntServer() {
  RemIntServer(INTB_VERTB, &VBlankInt);
}

int GetVBlankCounter() {
  return VBlankCounter / 2;
}

void SetVBlankCounter(int value) {
  VBlankCounter = value / 2;
}

void ChangeVBlankCounter(int value) {
  value /= 2;

  if ((value < 0) && (VBlankCounter < -value))
    VBlankCounter = 0;
  else
    VBlankCounter += value;
}
