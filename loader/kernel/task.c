#include <cpu.h>
#include <debug.h>
#include <string.h>
#include <strings.h>
#include <interrupt.h>
#include <task.h>

static TaskT MainTask;
TaskT *CurrentTask = &MainTask;

static TaskListT ReadyList = TAILQ_HEAD_INITIALIZER(ReadyList);
u_char NeedReschedule = 0;

void IntrEnable(void) {
  Assert(CurrentTask->intrNest > 0);
  if (--CurrentTask->intrNest == 0)
    CpuIntrEnable();
}

void IntrDisable(void) {
  CpuIntrDisable();
  CurrentTask->intrNest++;
}

void TaskInit(TaskT *tsk, const char *name, void *stkptr, u_int stksz) {
  bzero(tsk, sizeof(TaskT));
  strlcpy(tsk->name, name, MAX_TASK_NAME_SIZE);
  tsk->stack.lower = stkptr;
  tsk->stack.upper = stkptr + stksz;
}

/* When calling RTE the stack must look as follows:
 *
 *   +--------+---------------+
 *   | FORMAT | VECTOR OFFSET | (M68010 only)
 *   +--------+---------------+
 *   |  PROGRAM COUNTER LOW   |
 *   +------------------------+
 *   |  PROGRAM COUNTER HIGH  |
 *   +------------------------+
 *   |    STATUS REGISTER     | <-- stack pointer
 *   +------------------------+
 *
 * Remember that stack grows down!
 */

/* Macros for task stack initialization. */
#define PushLong(v)                                                            \
  { *--(u_int *)sp = (u_int)(v); }
#define PushWord(v)                                                            \
  { *--(u_short *)sp = (u_short)(v); }

void TaskInitStack(TaskT *tsk, void (*fn)(void *), void *arg) {
  void *sp = tsk->stack.upper;

  PushLong(0); /* last return address at the bottom of stack */

  /* Exception stack frame starts with the return address, unless we're running
   * on 68010 and above. Then we need to put format vector word on stack. */
  if (CpuModel >= CPU_68010)
    PushWord(0);       /* format vector */
  PushLong((u_int)fn); /* return address */
  PushWord(SR_S);      /* status register */

  sp -= 6 * sizeof(u_int); /* A6 to A1 */
  PushLong((u_int)arg);    /* A0 */
  sp -= 9 * sizeof(u_int); /* D7 to D0 and USP */

  tsk->currSP = sp;
}

static void ReadyAdd(TaskT *tsk) {
  TAILQ_INSERT_TAIL(&ReadyList, tsk, node);
}

static void ReadyRemove(TaskT *tsk) {
  TAILQ_REMOVE(&ReadyList, tsk, node);
}

static TaskT *ReadyChoose(void) {
  TaskT *tsk = NULL;
  if (!TAILQ_EMPTY(&ReadyList)) {
    tsk = TAILQ_FIRST(&ReadyList);
    TAILQ_REMOVE(&ReadyList, tsk, node);
  }
  return tsk;
}

void TaskResume(TaskT *tsk) {
  IntrDisable();
  {
    ReadyAdd(tsk);
  }
  IntrEnable();
}

void TaskSuspend(TaskT *tsk) {
  IntrDisable();
  {
    if (tsk != NULL)
      ReadyRemove(tsk);
    TaskSwitch(NULL);
  }
  IntrEnable();
}

void TaskSwitch(TaskT *curtsk) {
  Assert(IntrDisabled());
  Assert(CurrentTask->intrNest == 1);

  if (curtsk)
    ReadyAdd(curtsk);

  CurrentTask = NULL;

  while (!(curtsk = ReadyChoose()))
    CpuWait();

  CurrentTask = curtsk;
}
