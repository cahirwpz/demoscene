#ifndef __TASK_H__
#define __TASK_H__

#include <queue.h>
#include <types.h>

#define MAX_TASK_NAME_SIZE 16

typedef struct Task TaskT;
typedef TAILQ_HEAD(, Task) TaskListT;

#define TS_READY 0     /* running or on ready list */
#define TS_BLOCKED 1   /* on blocked list */
#define TS_SUSPENDED 2 /* doesn't belong to any list */

struct Task {
  /* Points to task context pushed on top of the stack. */
  void *currSP;
  TAILQ_ENTRY(Task) node; /* Ready tasks are stored on ReadyList. */
  u_char state;
  u_char prio;
  short intrNest;
  struct {
    void *lower; /* Lowest stack address. */
    void *upper; /* Highest stack address. */
  } stack;
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

static inline void TaskYield(void) { asm volatile("\ttrap\t#0\n"); }

/* Enable / disable all interrupts. Handle nested calls. */
void IntrDisable(void);
void IntrEnable(void);

#endif
