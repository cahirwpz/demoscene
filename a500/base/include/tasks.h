#ifndef __TASKS_H__
#define __TASKS_H__

void DumpTasks();

/* @brief Volunatily release CPU time.
 *
 * Scheduler will choose next task to run. If calling task is the only in READY
 * state this function will return immediately.
 */
void TaskYield();

/* @brief Suspends a task in wait for an event.
 * @param event is a list of tasks awaiting the event to happen.
 *
 * XXX Only call it from user mode! */
void TaskWait(struct List *event);

/* @brief Awakes all tasks waiting for given event.
 *
 * XXX Only call it from within interrupt context! */
void TaskSignal(struct List *event);

#endif
