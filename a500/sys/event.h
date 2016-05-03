#ifndef __EVENT_H__
#define __EVENT_H__

#include "common.h"
#include "keyboard.h"
#include "mouse.h"

typedef union Event {
  UBYTE type;
  KeyEventT key;
  MouseEventT mouse;
} EventT;

__regargs void PushEvent(EventT *event);
__regargs BOOL PopEvent(EventT *event);

#endif
