#ifndef __EVENT_H__
#define __EVENT_H__

#include "common.h"

typedef enum { EV_UNKNOWN, EV_KEY, EV_MOUSE, EV_GUI } EventTypeT;

union Widget;

typedef struct KeyEvent {
  EventTypeT type;
  UBYTE modifier;
  UBYTE code;
  char ascii;
} KeyEventT;

typedef struct MouseEvent {
  EventTypeT type;
  UBYTE button;
  WORD  x, y;
  BYTE  xrel, yrel;
} MouseEventT;

typedef struct GuiEvent {
  EventTypeT type;
  UBYTE action;
  union Widget *widget;
} GuiEventT;

typedef union Event {
  EventTypeT type;
  KeyEventT key;
  MouseEventT mouse;
  GuiEventT gui;
} EventT;

__regargs void PushEvent(EventT *event);
__regargs BOOL PopEvent(EventT *event);

#endif
