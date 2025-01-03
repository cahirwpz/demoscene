#include <custom.h>
#include <effect.h>
#include <uae.h>
#include <system/interrupt.h>
#include <system/task.h>

extern EffectT Effect;

#define BGTASK 0

#if BGTASK
static void BgLoop(__unused void *ptr) {
  Log("Inside background task!\n");
  for (;;) {
    custom->color[0] = 0xff0;
  }
}

static void StartBgTask(void) {
  static __aligned(8) char stack[256];
  static TaskT BgTask;

  TaskInit(&BgTask, "background", stack, sizeof(stack));
  TaskRun(&BgTask, 1, BgLoop, NULL);
}
#endif

static int VBlankISR(void) {
  if (Effect.VBlank)
      Effect.VBlank();
  return 0;
}

INTSERVER(VBlankInterrupt, 0, (IntFuncT)VBlankISR, NULL);

int main(void) {
  /* NOP that triggers fs-uae debugger to stop and inform GDB that it should
   * fetch segments locations to relocate symbol information read from file. */
  asm volatile("exg %d7,%d7");

  AddIntServer(INTB_VERTB, VBlankInterrupt);

#if BGTASK
  StartBgTask();
#endif

  EffectLoad(&Effect);
  EffectInit(&Effect);
  UaeWarpMode(0);
  EffectRun(&Effect);
  EffectKill(&Effect);
  EffectUnLoad(&Effect);

  RemIntServer(INTB_VERTB, VBlankInterrupt);

  return 0; 
}
