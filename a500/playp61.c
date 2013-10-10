#include <exec/memory.h>

#include "p61/p61.h"
#include "file.h"
#include "hardware.h"
#include "print.h"

static FileDataT *module;

void Load() {
  module = ReadFile("data/tempest-acidjazzed_evening.p61", MEMF_CHIP);
}

void Kill() {
  FreeFile(module);
}

void Main() {
  P61_Init(module->data, NULL, NULL);
  P61_ControlBlock.Play = 1;

  WaitMouse();

  P61_ControlBlock.Play = 0;
  P61_End();
}
