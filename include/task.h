#ifndef __TASK_H__
#define __TASK_H__

#include <types.h>
#include <queue.h>

#define MAX_TASK_NAME_SIZE 16

typedef struct Task TaskT;
typedef TAILQ_HEAD(, Task) TaskListT;

struct Task {
  /* Points to task context pushed on top of the stack. */
  void *currSP;
  TAILQ_ENTRY(Task) node; /* Ready tasks are stored on ReadyList. */
  short intrNest;
  struct {
    void *lower; /* Lowest stack address. */
    void *upper; /* Highest stack address. */
  } stack;
  char name[MAX_TASK_NAME_SIZE]; /* Task name (limited in size) */
};

/* Points to task currently running on the processor. */
extern TaskT *CurrentTask;

void InitFirstTask(const char *name);
void TaskInit(TaskT *tsk, const char *name, void *stack, u_int stksz);
void TaskRun(TaskT *tsk, void (*fn)(void *), void *arg);
void TaskResume(TaskT *tsk);
void TaskSuspend(TaskT *tsk);
void TaskSwitch(TaskT *curtsk);

#endif
