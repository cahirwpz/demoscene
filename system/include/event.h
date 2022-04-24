#ifndef __EVENT_H__
#define __EVENT_H__

#include "common.h"

typedef enum { EV_UNKNOWN, EV_KEY, EV_MOUSE, EV_GUI } EventTypeT;

union Widget;

typedef struct KeyEvent {
  EventTypeT type;
  u_char modifier;
  u_char code;
  char ascii;
} KeyEventT;

typedef struct MouseEvent {
  EventTypeT type;
  u_char button;
  short  x, y;
  char  xrel, yrel;
} MouseEventT;

typedef struct GuiEvent {
  EventTypeT type;
  u_char action;
  union Widget *widget;
} GuiEventT;

typedef union Event {
  EventTypeT type;
  KeyEventT key;
  MouseEventT mouse;
  GuiEventT gui;
} EventT;

void PushEventISR(EventT *event);
void PushEvent(EventT *event);
bool PopEvent(EventT *event);

#endif
