#include "tools/loopevent.h"
#include "system/input.h"

LoopEventT ReadLoopEvent() {
  InputEventT event; 

  while (EventQueuePop(&event)) {
    switch (event.ie_Class) {
      case IECLASS_RAWKEY:
        if (event.ie_Code & IECODE_UP_PREFIX) {
          switch (event.ie_Code & ~IECODE_UP_PREFIX) {
            case KEY_RETURN:
              return LOOP_TRIGGER;
            case KEY_SPACE:
              return LOOP_PAUSE;
            case KEY_RIGHT:
              return LOOP_NEXT;
            case KEY_LEFT:
              return LOOP_PREV;
            case KEY_ESCAPE:
              return LOOP_EXIT;
            default:
              break;
          }
        }

      default:
        break;
    }
  }

  return LOOP_CONTINUE;
}
