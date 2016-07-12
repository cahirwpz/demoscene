#include "interrupts.h"
#include "hardware.h"
#include "mouse.h"
#include "event.h"

/* data for port 1, pin 9 (right mouse button) */
#define DATLY (1 << 10)

typedef struct {
  BYTE xctr, yctr;
  WORD x, y;
  WORD button;

  WORD left;
  WORD right;
  WORD top;
  WORD bottom;
} MouseDataT;

static MouseDataT mouseData;

static inline BOOL GetMouseX(MouseDataT *mouse, MouseEventT *event) {
  BYTE xctr = custom->joy0dat & 0xff;
  BYTE xrel = xctr - mouse->xctr;
  WORD x = mouse->x;

  if (!xrel) {
    event->x = x;
    return FALSE;
  }

  x += xrel;

  if (x < mouse->left)
    x = mouse->left;
  if (x > mouse->right)
    x = mouse->right;

  event->x = x;
  event->xrel = xrel;

  mouse->x = x;
  mouse->xctr = xctr;

  return TRUE;
}

static inline BOOL GetMouseY(MouseDataT *mouse, MouseEventT *event) {
  BYTE yctr = custom->joy0dat >> 8;
  BYTE yrel = yctr - mouse->yctr;
  WORD y = mouse->y;

  if (!yrel) {
    event->y = y;
    return FALSE;
  }

  y += yrel;

  if (y < mouse->top)
    y = mouse->top;
  if (y > mouse->bottom)
    y = mouse->bottom;

  event->y = y;
  event->yrel = yrel;

  mouse->y = y;
  mouse->yctr = yctr;

  return TRUE;
}

static inline UBYTE ReadButtonState() {
  UBYTE state = 0;

	if (!(ciaa->ciapra & CIAF_GAMEPORT0))
    state |= LMB_PRESSED;
  if (!(custom->potinp & DATLY))
    state |= RMB_PRESSED;

  return state;
}

static inline BOOL GetMouseButton(MouseDataT *mouse, MouseEventT *event) {
  UBYTE button = ReadButtonState();
  UBYTE change = (mouse->button ^ button) & (LMB_PRESSED | RMB_PRESSED);

  if (!change)
    return FALSE;

  mouse->button = button;
  event->button = 0;

  if (change & LMB_PRESSED)
    event->button |= (button & LMB_PRESSED) ? LMB_PRESSED : LMB_RELEASED;

  if (change & RMB_PRESSED)
    event->button |= (button & RMB_PRESSED) ? RMB_PRESSED : RMB_RELEASED;
  
  return TRUE;
}

static __interrupt LONG MouseIntHandler() {
  MouseEventT event;
  MouseDataT *mouse = &mouseData;
  BOOL moveX, moveY;

  memset(&event, 0, sizeof(event));
  event.type = EV_MOUSE;

  /* Register mouse position change first. */
  moveX = GetMouseX(mouse, &event);
  moveY = GetMouseY(mouse, &event);

  if (moveX || moveY)
    PushEvent((EventT *)&event);

  /* After that a change in mouse button state. */
  if (GetMouseButton(mouse, &event)) {
    event.x = mouse->x;
    event.y = mouse->y;
    PushEvent((EventT *)&event);
  }

  return 0;
}

INTERRUPT(MouseInterrupt, -5, MouseIntHandler);

__regargs void MouseInit(WORD minX, WORD minY, WORD maxX, WORD maxY) {
  MouseDataT *mouse = &mouseData;

  mouse->left = minX;
  mouse->right = maxX;
  mouse->top = minY;
  mouse->bottom = maxY;

  mouse->x = minX;
  mouse->y = minY;
  mouse->xctr = custom->joy0dat & 0xff;
  mouse->yctr = custom->joy0dat >> 8;
  mouse->button = ReadButtonState();

  AddIntServer(INTB_VERTB, &MouseInterrupt);
}

void MouseKill() {
  RemIntServer(INTB_VERTB, &MouseInterrupt);
}
