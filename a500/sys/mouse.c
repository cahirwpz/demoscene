#include "mouse.h"
#include "interrupts.h"
#include "hardware.h"

/* data for port 1, pin 9 (right mouse button) */
#define DATLY (1 << 10)

typedef struct MouseData {
  BYTE xctr, yctr;
  WORD x, y;
  WORD button;

  WORD left;
  WORD right;
  WORD top;
  WORD bottom;
} MouseDataT;

#define QUEUELEN 32

typedef struct MouseQueue {
  MouseEventT events[QUEUELEN];
  UBYTE head, tail, used;
} MouseQueueT;

static MouseDataT mouseData;
static MouseQueueT mouseQueue;

static inline void PushMouseEvent(MouseQueueT *queue, MouseEventT *event) {
  if (queue->used < QUEUELEN) {
    queue->events[queue->tail] = *event;
    queue->tail = (queue->tail + 1) & (QUEUELEN - 1);
    queue->used++;
  }
}

static inline BOOL PopMouseEvent(MouseQueueT *queue, MouseEventT *event) {
  BOOL result = FALSE;

  Disable();

  if (queue->used > 0) {
    *event = queue->events[queue->head];
    queue->head = (queue->head + 1) & (QUEUELEN - 1);
    queue->used--;
    result = TRUE;
  }

  Enable();

  return result;
}

static inline BOOL GetMouseX(MouseDataT *mouse, MouseEventT *event) {
  BYTE xctr = custom->joy0dat & 0xff;
  BYTE xrel = xctr - mouse->xctr;
  WORD x = mouse->x;

  if (!xrel)
    return FALSE;

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

  if (!yrel)
    return FALSE;

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

static inline MouseButtonT ReadButtonState() {
  MouseButtonT state = 0;

	if (!(ciaa->ciapra & CIAF_GAMEPORT0))
    state |= LMB_PRESSED;
  if (!(custom->potinp & DATLY))
    state |= RMB_PRESSED;

  return state;
}

static inline BOOL GetMouseButton(MouseDataT *mouse, MouseEventT *event) {
  MouseButtonT button = ReadButtonState();
  MouseButtonT change = (mouse->button ^ button) & (LMB_PRESSED | RMB_PRESSED);

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
  MouseDataT *mouse = &mouseData;
  MouseQueueT *queue = &mouseQueue;
  MouseEventT event;

  /* Register mouse position change first. */
  BOOL moveX = GetMouseX(mouse, &event);
  BOOL moveY = GetMouseY(mouse, &event);

  if (moveX || moveY) {
    event.button = mouse->button;
    PushMouseEvent(queue, &event);
  }

  /* After that a change in mouse button state. */
  if (GetMouseButton(mouse, &event))
    PushMouseEvent(queue, &event);

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

__regargs BOOL GetMouseEvent(MouseEventT *event) {
  return PopMouseEvent(&mouseQueue, event);
}
