#include <proto/alib.h>
#include <proto/exec.h>

#include "tasks.h"
#include "interrupts.h"
#include "effect.h"

#define IDLETASK 0

#if IDLETASK
static void IdleTask(void) {
  for (;;) {
    custom->color[0] = 0x00f;
  }
}
#endif

/* VBlank event list. */
static struct List *VBlankEvent = &(struct List){NULL, NULL, NULL, 0, 0};

/* Wake up tasks asleep in wait for VBlank interrupt. */
static int VBlankEventHandler(void) {
  TaskSignalIntr(VBlankEvent);
  return 0;
}

/* Puts a task into sleep waiting for VBlank interrupt. */
void TaskWaitVBlank(void) {
  TaskWait(VBlankEvent);
}

INTERRUPT(VBlankWakeUp, 10, VBlankEventHandler, NULL);

void InitVBlank(void) {
#if IDLETASK
  struct Task *idleTask = CreateTask("IdleTask", -10, IdleTask, 1024);
#endif

  NewList(VBlankEvent);
  AddIntServer(INTB_VERTB, VBlankWakeUp);
}

void KillVBlank(void) {
#if IDLETASK
  RemTask(idleTask);
#endif

  RemIntServer(INTB_VERTB, VBlankWakeUp);
}
