#ifndef __TASKS_H__
#define __TASKS_H__

#include <exec/lists.h>

void DumpTasks();

/* @brief Volunatily release CPU time.
 *
 * Scheduler will choose next task to run. If calling task is the only in READY
 * state this function will return immediately.
 */
void TaskYield();

/* @brief Suspends a task in wait for an event.
 * @param event is a list of tasks awaiting the event to happen.
 * @note Only call it from user mode! */
void TaskWait(struct List *event asm("a0"));

/* @brief Awakes all tasks waiting for given event.
 * @note Only call it from user mode! */
void TaskSignal(struct List *event asm("a0"));

/* @brief Awakes all tasks waiting for given event.
 * @note Only call it from within interrupt context! */
void TaskSignalIntr(struct List *event asm("a0"));

#endif
