#ifndef __SYSTEM_EVENT_QUEUE_H__
#define __SYSTEM_EVENT_QUEUE_H__

#include <devices/inputevent.h>

#include "std/types.h"

typedef struct InputEvent InputEventT;

void StartEventQueue();
void StopEventQueue();

void EventQueueReset();
bool EventQueuePop(InputEventT *event);

#endif
