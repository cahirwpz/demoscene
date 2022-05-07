#include <custom.h>
#include <effect.h>
#include <system/interrupt.h>
#include <system/task.h>

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

#define BGTASK 0

#if BGTASK
static void BgLoop(__unused void *ptr) {
  Log("Inside background task!\n");
  for (;;) {
    custom->color[0] = 0xff0;
  }
}
#endif

static void StartBgTask(void) {
#if BGTASK
  static __aligned(8) char stack[256];
  static TaskT BgTask;

  TaskInit(&BgTask, "background", stack, sizeof(stack));
  TaskRun(&BgTask, 1, BgLoop, NULL);
#endif
}

int main(void) {
  /* NOP that triggers fs-uae debugger to stop and inform GDB that it should
   * fetch segments locations to relocate symbol information read from file. */
  asm volatile("exg %d7,%d7");

  StartBgTask();

  AddIntServer(INTB_VERTB, VertBlankWakeup);

  EffectLoad(&Effect);
  EffectInit(&Effect);
  EffectRun(&Effect);
  EffectKill(&Effect);
  EffectUnLoad(&Effect);

  RemIntServer(INTB_VERTB, VertBlankWakeup);

  return 0;
}
