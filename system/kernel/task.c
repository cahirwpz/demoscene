#include <debug.h>
#include <common.h>
#include <string.h>
#include <strings.h>
#include <system/cpu.h>
#include <system/task.h>

static TaskT MainTask;
TaskT *CurrentTask = &MainTask;

static TaskListT ReadyList = TAILQ_HEAD_INITIALIZER(ReadyList);
static TaskListT WaitList = TAILQ_HEAD_INITIALIZER(WaitList);
u_char NeedReschedule = 0;

static __unused const char *TaskState[] = {
  [TS_READY] = "READY",
  [TS_BLOCKED] = "BLOCKED",
  [TS_SUSPENDED] = "SUSPENDED"
};

#define TI_FMT "'%s:%d' (%s, $%08x)"
#define TI_ARGS(tsk)                                                          \
  (tsk)->name, (tsk)->prio, TaskState[(tsk)->state], (tsk)->eventSet

/* If this gets overwritten you need to allocate more stack. */
#define STACK_CANARY 0xdeadc0de

void TaskInit(TaskT *tsk, const char *name, void *stkptr, u_int stksz) {
  bzero(tsk, sizeof(TaskT));
  strlcpy(tsk->name, name, MAX_TASK_NAME_SIZE);
  tsk->state = (tsk == CurrentTask) ? TS_READY : TS_SUSPENDED;
  tsk->stkLower = stkptr;
  tsk->stkUpper = stkptr + stksz;
  *(u_int *)stkptr = STACK_CANARY;
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
#define PushLong(v) stld(sp, v)
#define PushWord(v) stwd(sp, v)

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

  tsk->ctx = sp;
  tsk->prio = prio;
  TaskResume(tsk);
}

void ReadyAdd(TaskT *tsk) {
  if (TAILQ_EMPTY(&ReadyList)) {
    TAILQ_INSERT_HEAD(&ReadyList, tsk, node);
  } else {
    TaskT *before = TAILQ_FIRST(&ReadyList);
    /* Insert before first task with lower priority.
    * Please note that 0 is the highest priority! */
    while (before != NULL && before->prio <= tsk->prio)
      before = TAILQ_NEXT(before, node);
    if (before == NULL)
      TAILQ_INSERT_TAIL(&ReadyList, tsk, node);
    else
      TAILQ_INSERT_BEFORE(before, tsk, node);
  }

  tsk->state = TS_READY;
}

/* Take a task with highest priority. */
static TaskT *ReadyChoose(void) {
  TaskT *tsk;
  Assume(GetIPL() == IPL_MAX);
  tsk = TAILQ_FIRST(&ReadyList);
  Assume(tsk != NULL);
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
  Assume(ipl > IPL_NONE);
  Assume(tsk->state == TS_SUSPENDED);
  ReadyAdd(tsk);
  MaybePreemptISR();
  (void)SetIPL(ipl);
}

/* Preemption from task context is performed by trap handler that executes
 * YieldHandler procedure. */
void MaybePreempt(void) {
  TaskT *first = TAILQ_FIRST(&ReadyList);
  if (first == NULL)
    return;
  if (first->prio >= CurrentTask->prio)
    return;
  TaskYield();
}

void TaskResume(TaskT *tsk) {
  IntrDisable();
  Assume(tsk->state == TS_SUSPENDED);
  ReadyAdd(tsk);
  MaybePreempt();
  IntrEnable();
}

void TaskSuspend(TaskT *tsk) {
  IntrDisable();
  Assume(tsk->state == TS_READY);
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
    Assume(tsk->state == TS_READY);
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
  Assume(eventSet != 0);
  IntrDisable();
  tsk->waitpt = __builtin_return_address(0);
  tsk->eventSet = eventSet;
  tsk->state = TS_BLOCKED;
  TAILQ_REMOVE(&ReadyList, tsk, node);
  TAILQ_INSERT_HEAD(&WaitList, tsk, node);
  Debug("Task " TI_FMT " is going to sleep.", TI_ARGS(tsk));
  TaskYield();
  eventSet = tsk->eventSet;
  tsk->waitpt = NULL;
  tsk->eventSet = 0;
  IntrEnable();
  return eventSet;
}

static int _TaskNotify(u_int eventSet) {
  TaskT *tsk, *tmp;
  int ntasks = 0;
  Assume(eventSet != 0);
  TAILQ_FOREACH_SAFE(tsk, &WaitList, node, tmp) {
    if (tsk->eventSet & eventSet) {
      Debug("Waking up " TI_FMT " (got $%08x).", TI_ARGS(tsk), eventSet);
      tsk->eventSet &= eventSet;
      TAILQ_REMOVE(&WaitList, tsk, node);
      ReadyAdd(tsk);
      ntasks++;
    }
  }
  return ntasks;
}

int TaskNotifyISR(u_int eventSet) {
  int res;
  u_short ipl = SetIPL(SR_IM);
  Assume(ipl > IPL_NONE);
  Debug("New events: $%08x", eventSet);
  if ((res = _TaskNotify(eventSet)))
    MaybePreemptISR();
  (void)SetIPL(ipl);
  return res;
}

int TaskNotify(u_int eventSet) {
  int res;
  Assume(GetIPL() == IPL_NONE);
  IntrDisable();
  Debug("New events: $%08x", eventSet);
  if ((res = _TaskNotify(eventSet)))
    MaybePreempt();
  IntrEnable();
  return res;
}

void TaskSwitch(TaskT *curtsk) {
  Assume(GetIPL() == IPL_MAX);
  Assume(curtsk != NULL);
  Debug("Switching from " TI_FMT ".", TI_ARGS(curtsk));
  if (curtsk->state == TS_READY)
    ReadyAdd(curtsk);
  curtsk = ReadyChoose();
  Debug("Switching to " TI_FMT ".", TI_ARGS(curtsk));
  if (*(u_int *)curtsk->stkLower != STACK_CANARY) {
    Panic("[TaskSwitch] Stack overflow detected for '%s' task (size: %d)!",
          curtsk->name, (int)(curtsk->stkUpper - curtsk->stkLower));
  }
  CurrentTask = curtsk;
}

void TaskDebug(void) {
  TaskT *curtsk = CurrentTask;
  TaskT *tsk;

  Log("[Task] (*) PC: $%08x SR: $%04x " TI_FMT ".\n",
      curtsk->ctx->pc, curtsk->ctx->sr, TI_ARGS(curtsk));
  if (*(u_int *)curtsk->stkLower != STACK_CANARY) {
    Log("Stack overflow detected (size: %d)!\n",
        (int)(curtsk->stkUpper - curtsk->stkLower));
  }
  if (!TAILQ_EMPTY(&ReadyList)) {
    TAILQ_FOREACH(tsk, &ReadyList, node) {
      Log("[Task] (R) PC: $%08x SR: $%04x " TI_FMT "\n",
          tsk->ctx->pc, tsk->ctx->sr, TI_ARGS(tsk));
    }
  }
  if (!TAILQ_EMPTY(&WaitList)) {
    TAILQ_FOREACH(tsk, &WaitList, node) {
      Log("[Task] (W) waitpt: $%08x " TI_FMT "\n",
          (u_int)tsk->waitpt, TI_ARGS(tsk));
    }
  }
}
