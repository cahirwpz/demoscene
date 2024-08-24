#ifndef __SYSTEM_TASK_H__
#define __SYSTEM_TASK_H__

#include <types.h>
#include <system/queue.h>

#define MAX_TASK_NAME_SIZE 16

typedef struct Task TaskT;
typedef TAILQ_HEAD(, Task) TaskListT;

#define TS_READY 0     /* running or on ready list */
#define TS_BLOCKED 1   /* on blocked list */
#define TS_SUSPENDED 2 /* doesn't belong to any list */

/* Task event flags.
 *
 * Lower event bits map to hardware interrupts:
 * - 13...0 coming from custom chipset,
 * - 18...14 coming from CIA A,
 * - 23...19 coming from CIA B.
 * Upper 8 bits can be used by user to synchronize tasks.
 *
 * You can directly use INTF_* constant to specify events you're waiting for. */
#define EVF_CUSTOM(x) (x)
#define EVF_CIAA(x) ((x) << 14)
#define EVF_CIAB(x) ((x) << 19)
#define EVF_SWI(x) ((x) << 23)

struct Task {
  void *currSP; /* Points to task context pushed on top of the stack. */
  TAILQ_ENTRY(Task) node; /* Ready tasks are stored on ReadyList. */
  u_char state;           /* Task state - one of TS_* constants. */
  u_char prio;    /* Task priority - 0 is the highest, 255 is the lowest. */
  short intrNest; /* Interrupt disable nesting count. */
  u_int eventSet; /* Events we're waiting for - combination of EVF_* flags. */
  void *stkLower; /* Lowest stack address. */
  void *stkUpper; /* Highest stack address. */
  char name[MAX_TASK_NAME_SIZE]; /* Task name (limited in size) */
};

/* Points to task currently running on the processor. */
extern TaskT *CurrentTask;

void TaskInit(TaskT *tsk, const char *name, void *stkptr, u_int stksz);
void TaskRun(TaskT *tsk, u_char prio, void (*fn)(void *), void *arg);
void TaskResume(TaskT *tsk);
void TaskResumeISR(TaskT *tsk);
void TaskSuspend(TaskT *tsk);
void TaskPrioritySet(TaskT *tsk, u_char prio);

#ifdef _TASK_PRIVATE
void ReadyAdd(TaskT *tsk);
#endif

u_int TaskWait(u_int eventSet);
int TaskNotifyISR(u_int eventSet);
int TaskNotify(u_int eventSet);

static inline void TaskYield(void) { asm volatile("trap #0"); }

/* Enable / disable all interrupts. Handle nested calls. */
void IntrDisable(void);
void IntrEnable(void);

#endif /* !__SYSTEM_TASK_H__ */
