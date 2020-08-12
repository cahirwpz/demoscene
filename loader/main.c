#include "startup.h"
#include "memory.h"
#include "io.h"
#include "hardware.h"
#include "sync.h"
#include "effect.h"

extern EffectT Effect;

/* TODO Puts a task into sleep waiting for VBlank interrupt. */
void TaskWaitVBlank(void) {
  WaitVBlank();
}

int main(void) {
  SystemInfo();
  KillOS();
  BootMemory();

  InitFloppyIO();
  InitTracks();

  EffectLoad(&Effect);
  EffectInit(&Effect);
  EffectRun(&Effect);
  EffectKill(&Effect);
  EffectUnLoad(&Effect);

  KillFloppyIO();

  RestoreOS();

  return 0;
}
