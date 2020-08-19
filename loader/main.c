#include <custom.h>

#include "sync.h"
#include "effect.h"

extern EffectT Effect;

/* TODO Puts a task into sleep waiting for VBlank interrupt. */
void TaskWaitVBlank(void) {
  WaitVBlank();
}

int main(void) {
  NOP();

  EffectLoad(&Effect);
  EffectInit(&Effect);
  EffectRun(&Effect);
  EffectKill(&Effect);
  EffectUnLoad(&Effect);

  return 0;
}
