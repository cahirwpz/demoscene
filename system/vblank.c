#include <debug.h>
#include <system/interrupt.h>
#include <system/task.h>
#include <linkerset.h>

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

void InitVBlank(void) {
  Log("[VBlank] Registered a wakeup handler.\n");
  AddIntServer(INTB_VERTB, VertBlankWakeup);
}

void KillVBlank(void) {
  RemIntServer(INTB_VERTB, VertBlankWakeup);
}

ADD2INIT(InitVBlank, 0);
ADD2EXIT(KillVBlank, 0);
