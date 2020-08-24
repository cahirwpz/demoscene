#include <string.h>
#include <interrupt.h>
#include <cia.h>
#include <mouse.h>
#include <event.h>

typedef struct {
  char xctr, yctr;
  short x, y;
  short button;

  short left;
  short right;
  short top;
  short bottom;
} MouseDataT;

static MouseDataT mouseData;

static inline bool GetMouseX(MouseDataT *mouse, MouseEventT *event) {
  char xctr = custom->joy0dat & 0xff;
  char xrel = xctr - mouse->xctr;
  short x = mouse->x;

  if (!xrel) {
    event->x = x;
    return false;
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

  return true;
}

static inline bool GetMouseY(MouseDataT *mouse, MouseEventT *event) {
  char yctr = custom->joy0dat >> 8;
  char yrel = yctr - mouse->yctr;
  short y = mouse->y;

  if (!yrel) {
    event->y = y;
    return false;
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

  return true;
}

static inline u_char ReadButtonState(void) {
  u_char state = 0;

	if (!(ciaa->ciapra & CIAF_GAMEPORT0))
    state |= LMB_PRESSED;
  if (!(custom->potinp & DATLY))
    state |= RMB_PRESSED;

  return state;
}

static inline bool GetMouseButton(MouseDataT *mouse, MouseEventT *event) {
  u_char button = ReadButtonState();
  u_char change = (mouse->button ^ button) & (LMB_PRESSED | RMB_PRESSED);

  if (!change)
    return false;

  mouse->button = button;
  event->button = 0;

  if (change & LMB_PRESSED)
    event->button |= (button & LMB_PRESSED) ? LMB_PRESSED : LMB_RELEASED;

  if (change & RMB_PRESSED)
    event->button |= (button & RMB_PRESSED) ? RMB_PRESSED : RMB_RELEASED;
  
  return true;
}

static int MouseIntHandler(void) {
  MouseEventT event;
  MouseDataT *mouse = &mouseData;
  bool moveX, moveY;

  memset(&event, 0, sizeof(event));
  event.type = EV_MOUSE;

  /* Register mouse position change first. */
  moveX = GetMouseX(mouse, &event);
  moveY = GetMouseY(mouse, &event);

  if (moveX || moveY)
    PushEventISR((EventT *)&event);

  /* After that a change in mouse button state. */
  if (GetMouseButton(mouse, &event)) {
    event.x = mouse->x;
    event.y = mouse->y;
    PushEventISR((EventT *)&event);
  }

  return 0;
}

INTSERVER(MouseServer, -5, (IntFuncT)MouseIntHandler, NULL);

void MouseInit(short minX, short minY, short maxX, short maxY) {
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

  AddIntServer(VertBlankChain, MouseServer);
}

void MouseKill(void) {
  RemIntServer(VertBlankChain, MouseServer);
}
