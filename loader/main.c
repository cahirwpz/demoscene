#include "startup.h"
#include "memory.h"
#include "io.h"
#include "hardware.h"
#include "sync.h"
#include "effect.h"

extern EffectT Effect;

int main(void) {
  SystemInfo();
  KillOS();

  InitMemory();
  InitFloppyIO();
  InitTracks();
  InitVBlank();

  EffectLoad(&Effect);
  EffectInit(&Effect);
  EffectRun(&Effect);
  EffectKill(&Effect);
  EffectUnLoad(&Effect);

  KillVBlank();
  KillFloppyIO();
  KillMemory();

  RestoreOS();

  return 0;
}
