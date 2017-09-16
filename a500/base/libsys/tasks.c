#include <exec/execbase.h>
#include <exec/tasks.h>
#include <proto/exec.h>

#include "common.h"
#include "tasks.h"

static void DumpTaskList(struct List *list) {
  struct Node *node = list->lh_Head;

  for (;;) {
    Log("> %lx (%ld) '%s'\n", (LONG)node, (LONG)node->ln_Pri, node->ln_Name);
    if (node == list->lh_TailPred)
      break;
    node = node->ln_Succ;
  }
}

void DumpTasks() {
  struct List *list;
  struct Node *node = (struct Node *)FindTask(NULL);

  Log("Task running:\n");
  Log("> %lx (%ld) '%s'\n", (LONG)node, (LONG)node->ln_Pri, node->ln_Name);
 
  list = &SysBase->TaskReady;
  if (!IsListEmpty(list)) {
    Log("Ready tasks:\n");
    DumpTaskList(list);
  }

  list = &SysBase->TaskWait;
  if (!IsListEmpty(list)) {
    Log("Waiting tasks:\n");
    DumpTaskList(list);
  }
}

/* SysBase::SysFlags undocumented flags related to scheduling. */
#define SF_SAR  (1 << 15) /* Scheduler attention request */
#define SF_TQE  (1 << 14) /* Time quantum elapsed */

/* Please read exec.library disassembly by Markus Wandel
 * to learn about undocumented functions like Switch() and Schedule(). */

void TaskYield() {
  struct Task *task = SysBase->ThisTask;
  Disable();
  /* Mark this task as ready to be run. */
  task->tc_State = TS_READY;
  /* Put it on a ready task queue. */
  Enqueue(&SysBase->TaskReady, &task->tc_Node);
  /* Force a task switch by calling Switch routine in supervisor mode. */
  Supervisor((void *)SysBase - 0x36);
  Enable();
}

/* Waiting tasks are not put into SysBase->TaskWait list! */
void TaskWait(struct List *event) {
  struct Task *task = SysBase->ThisTask;
  Disable();
  /* Mark this task as suspended. */
  task->tc_State = TS_WAIT;
  /* Put it on a event wait list. */
  AddTail(event, &task->tc_Node);
  /* Force a task switch by calling Switch routine in supervisor mode. */
  Supervisor((void *)SysBase - 0x36);
  Enable();
}

void TaskSignal(struct List *event) {
  struct Node *node;
  /* Remove each task from the list. */
  while ((node = RemHead(event))) {
    struct Task *task = (struct Task *)node;
    /* Mark it as ready to run. */
    task->tc_State = TS_READY;
    /* Put it on a ready task queue. */
    Enqueue(&SysBase->TaskReady, &task->tc_Node);
    /* Force a scheduler to take action while returning from interrupt! */
    SysBase->SysFlags |= SF_SAR;
  }
}
