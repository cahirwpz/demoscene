#include <task.h>

TaskT *CurrentTask = NULL;
u_char NeedReschedule = 0;

void TaskSwitch(__unused TaskT *curtask) {
}
