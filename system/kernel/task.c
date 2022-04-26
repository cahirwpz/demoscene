#include <debug.h>
#include <string.h>
#include <strings.h>
#include <system/cpu.h>
#include <system/task.h>

#define DEBUG 0

#if DEBUG
#define Debug(fmt, ...) Log("[%s] " fmt "\n", __func__, __VA_ARGS__)
#else
#define Debug(fmt, ...) ((void)0)
#endif

static TaskT MainTask;
TaskT *CurrentTask = &MainTask;

static TaskListT ReadyList = TAILQ_HEAD_INITIALIZER(ReadyList);
static TaskListT WaitList = TAILQ_HEAD_INITIALIZER(WaitList);
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
  tsk->state = (tsk == CurrentTask) ? TS_READY : TS_SUSPENDED;
  tsk->stkLower = stkptr;
  tsk->stkUpper = stkptr + stksz;
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
  void *sp = tsk->stkUpper;

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

void ReadyAdd(TaskT *tsk) {
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
  Assert(GetIPL() == IPL_MAX);
  tsk = TAILQ_FIRST(&ReadyList);
  if (tsk != NULL)
    TAILQ_REMOVE(&ReadyList, tsk, node);
  return tsk;
}

/* Preemption from interrupt context is performed in LeaveIntr
 * when NeedReschedule is set. */
static void MaybePreemptISR(void) {
  TaskT *first = TAILQ_FIRST(&ReadyList);
  if (first == NULL)
    return;
  if (first->prio >= CurrentTask->prio)
    return;
  NeedReschedule = -1;
}

void TaskResumeISR(TaskT *tsk) {
  u_short ipl = SetIPL(SR_IM);
  Assert(ipl > IPL_NONE);
  Assert(tsk->state == TS_SUSPENDED);
  ReadyAdd(tsk);
  MaybePreemptISR();
  (void)SetIPL(ipl);
}

/* Preemption from task context is performed by trap handler that executes
 * YieldHandler procedure. */
static void MaybePreempt(void) {
  TaskT *first = TAILQ_FIRST(&ReadyList);
  if (first == NULL)
    return;
  if (first->prio >= CurrentTask->prio)
    return;
  TaskYield();
}

void TaskResume(TaskT *tsk) {
  IntrDisable();
  Assert(tsk->state == TS_SUSPENDED);
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
  TaskYield();
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

u_int TaskWait(u_int eventSet) {
  TaskT *tsk = CurrentTask;
  Assert(eventSet != 0);
  IntrDisable();
  tsk->eventSet = eventSet;
  tsk->state = TS_BLOCKED;
  TAILQ_INSERT_HEAD(&WaitList, tsk, node);
  Debug("Task '%s' waits for %08x events.", tsk->name, eventSet);
  TaskYield();
  eventSet = tsk->eventSet;
  tsk->eventSet = 0;
  IntrEnable();
  return eventSet;
}

static int _TaskNotify(u_int eventSet) {
  TaskT *tsk;
  int ntasks = 0;
  Assert(eventSet != 0);
  TAILQ_FOREACH(tsk, &WaitList, node) {
    if (tsk->eventSet & eventSet) {
      Debug("Waking up '%s' task waiting on $%08x (got $%08x).",
            tsk->name, tsk->eventSet, eventSet);
      tsk->eventSet &= eventSet;
      ReadyAdd(tsk);
      ntasks++;
    }
  }
  if (ntasks == 0)
    Log("[TaskNotify] Nobody was waiting for %08x events!\n", eventSet);
  return ntasks;
}

void TaskNotifyISR(u_int eventSet) {
  u_short ipl = SetIPL(SR_IM);
  Assert(ipl > IPL_NONE);
  if (_TaskNotify(eventSet))
    MaybePreemptISR();
  (void)SetIPL(ipl);
}

void TaskNotify(u_int eventSet) {
  Assert(GetIPL() == IPL_NONE);
  IntrDisable();
  if (_TaskNotify(eventSet))
    MaybePreempt();
  IntrEnable();
}

void TaskSwitch(TaskT *curtsk) {
  Assert(GetIPL() == IPL_MAX);
  Assert(curtsk != NULL);
  if (curtsk->state == TS_READY)
    ReadyAdd(curtsk);
  while (!(curtsk = ReadyChoose())) {
    Debug("Processor goes asleep with SR=%04x!", 0x2000);
    CpuWait();
    CpuIntrDisable();
  }
  Debug("Switching to '%s', prio: %d.", curtsk->name, curtsk->prio);
  CurrentTask = curtsk;
}
