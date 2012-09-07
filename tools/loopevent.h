#ifndef __TOOLS_LOOP_EVENT_H__
#define __TOOLS_LOOP_EVENT_H__

typedef enum {
  LOOP_CONTINUE, LOOP_EXIT, LOOP_TRIGGER, LOOP_NEXT, LOOP_PREV
} LoopEventT;

LoopEventT ReadLoopEvent();

#endif
