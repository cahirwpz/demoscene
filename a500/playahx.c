#include "file.h"
#include "hardware.h"
#include "memory.h"
#include "ahx/ahx.h"
#include "interrupts.h"

static void *module;

void Load() {
  module = ReadFile("data/jazzcat-electric_city.ahx", MEMF_PUBLIC);
}

void Kill() {
  FreeAutoMem(module);
}

__saveds void MyAhxInterrupt() {
  custom->color[0] = 0;
  AhxInterrupt();
  custom->color[0] = 0xaaa;
}

void Main() {
  custom->intena = INTF_SETCLR | INTF_SOFTINT | INTF_PORTS | INTF_INTEN;
  custom->dmacon = DMAF_SETCLR | DMAF_MASTER;

  if (AhxInitCIA(MyAhxInterrupt, 0) == 0) {
    if (AhxInitPlayer(NULL, NULL, 0, 0) == 0) {
      if (AhxInitModule(module) == 0) {
        AhxInitSubSong(0, 0);
        WaitMouse();
        AhxStopSong();
      }
      AhxKillPlayer();
    }
    AhxKillCIA();
  }
}
