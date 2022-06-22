#include <custom.h>
#include <effect.h>
#include <system/amigahunk.h>
#include <system/file.h>
#include <system/filesys.h>
#include <system/interrupt.h>
#include <system/task.h>

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

static void DoEffect(FileT *fh) {
  HunkT *hl = LoadHunkList(fh);
  EffectT *(*entry)(void) = (void *)hl->data;
  EffectT *effect = entry();

  EffectLoad(effect);
  EffectInit(effect);
  EffectRun(effect);
  EffectKill(effect);
  EffectUnLoad(effect);

  FreeHunkList(hl);
}

int main(void) {
  /* NOP that triggers fs-uae debugger to stop and inform GDB that it should
   * fetch segments locations to relocate symbol information read from file. */
  asm volatile("exg %d7,%d7");

  StartBgTask();

  AddIntServer(INTB_VERTB, VertBlankWakeup);

  {
    const FileEntryT *fe = NULL;
    FileT *fh;

    while (FileSysList(&fe)) {
      if (!strcmp(fe->name, "system.exe"))
        continue;
      if (fe->type != FE_EXEC)
        continue;
      fh = OpenFileEntry(fe);
      DoEffect(fh);
      FileClose(fh);
    }
  }

  RemIntServer(INTB_VERTB, VertBlankWakeup);

  return 0;
}
