#include <cpu.h>
#include <debug.h>
#include <string.h>
#include <strings.h>
#include <task.h>

static TaskT MainTask;
TaskT *CurrentTask = &MainTask;

static TaskListT ReadyList = TAILQ_HEAD_INITIALIZER(ReadyList);
u_char NeedReschedule = 0;

void TaskSwitch(TaskT *curtsk);

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
  tsk->state = (tsk == CurrentTask) ? TS_READY : TS_SUSPENDED;
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

void TaskRun(TaskT *tsk, u_char prio, void (*fn)(void *), void *arg) {
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
  tsk->prio = prio;
  TaskResume(tsk);
}

static void ReadyAdd(TaskT *tsk) {
  TaskT *before = TAILQ_FIRST(&ReadyList);
  /* Insert before first task with lower priority.
   * Please note that 0 is the highest priority! */
  while (before != NULL && before->prio <= tsk->prio)
    before = TAILQ_NEXT(before, node);
  if (before == NULL)
    TAILQ_INSERT_TAIL(&ReadyList, tsk, node);
  else
    TAILQ_INSERT_BEFORE(before, tsk, node);
  tsk->state = TS_READY;
}

/* Take a task with highest priority. */
static TaskT *ReadyChoose(void) {
  TaskT *tsk;
  Assert(CpuIntrDisabled());
  tsk = TAILQ_FIRST(&ReadyList);
  if (tsk != NULL)
    TAILQ_REMOVE(&ReadyList, tsk, node);
  return tsk;
}

/* Preemption from interrupt context is performed in LeaveIntr
 * when NeedReschedule is set. */
static void MaybePreemptISR(void) {
  TaskT *first = TAILQ_FIRST(&ReadyList);
  if (first != NULL && CurrentTask->prio < first->prio)
    NeedReschedule = -1;
}

void TaskResumeISR(TaskT *tsk) {
  Assert(CpuIntrDisabled());
  Assert(tsk->state != TS_READY);
  ReadyAdd(tsk);
  MaybePreemptISR();
}

/* Preemption from task context is performed by trap handler that executes
 * YieldHandler procedure. */
static void MaybePreempt(void) {
  TaskT *first = TAILQ_FIRST(&ReadyList);
  if (first != NULL && CurrentTask->prio < first->prio)
    TaskYield();
}

void TaskResume(TaskT *tsk) {
  IntrDisable();
  Assert(tsk->state != TS_READY);
  ReadyAdd(tsk);
  MaybePreempt();
  IntrEnable();
}

void TaskSuspend(TaskT *tsk) {
  IntrDisable();
  Assert(tsk->state == TS_READY);
  if (tsk != NULL) {
    TAILQ_REMOVE(&ReadyList, tsk, node);
    tsk->state = TS_SUSPENDED;
  } else {
    tsk = CurrentTask;
  }
  tsk->state = TS_SUSPENDED;
  TaskSwitch(NULL);
  IntrEnable();
}

void TaskPrioritySet(TaskT *tsk, u_char prio) {
  IntrDisable();
  if (tsk != NULL) {
    Assert(tsk->state == TS_READY);
    TAILQ_REMOVE(&ReadyList, tsk, node);
  } else {
    tsk = CurrentTask;
  }
  tsk->prio = prio;
  ReadyAdd(tsk);
  MaybePreempt();
  IntrEnable();
}

void TaskSwitch(TaskT *curtsk) {
  Assert(CpuIntrDisabled());
  if (curtsk)
    ReadyAdd(curtsk);
  CurrentTask = NULL;
  while (!(curtsk = ReadyChoose()))
    CpuWait();
  CurrentTask = curtsk;
}
