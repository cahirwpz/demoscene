#include <custom.h>
#include <interrupt.h>
#include <task.h>

#include "sync.h"
#include "effect.h"

extern EffectT Effect;

static u_char IsWaiting = 0;

static void VBlankWakeupHandler(void) {
  if (IsWaiting) {
    IsWaiting = 0;
    TaskNotifyISR(INTF_VERTB);
  }
}

INTSERVER(VertBlankWakeup, 0, (IntFuncT)VBlankWakeupHandler, NULL);

/* Puts a task into sleep waiting for VBlank interrupt. */
void TaskWaitVBlank(void) {
  IntrDisable();
  IsWaiting = -1;
  TaskWait(INTF_VERTB);
  IntrEnable();
}

int main(void) {
  AddIntServer(VertBlankChain, VertBlankWakeup);

  EffectLoad(&Effect);
  EffectInit(&Effect);
  EffectRun(&Effect);
  EffectKill(&Effect);
  EffectUnLoad(&Effect);

  RemIntServer(VertBlankChain, VertBlankWakeup);

  return 0;
}
