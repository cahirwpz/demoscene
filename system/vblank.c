#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <proto/exec.h>

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
  return VBlankCounter;
}

void SetVBlankCounter(int value) {
  VBlankCounter = value;
}
