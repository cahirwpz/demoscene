#include <custom.h>
#include <debug.h>
#include <types.h>
#include <system/cia.h>
#include <system/event.h>
#include <system/interrupt.h>
#include <system/mouse.h>

typedef struct {
  MouseEventT event;

  char xctr, yctr;
  u_char button;

  short left;
  short right;
  short top;
  short bottom;
} MouseDataT;

static MouseDataT MouseData;

static bool GetMouseMove(MouseDataT *mouse) {
  u_short joy0dat = custom->joy0dat;
  char xctr, xrel, yctr, yrel;

  xctr = joy0dat;
  xrel = xctr - mouse->xctr;

  if (xrel) {
    short x = mouse->event.x + xrel;

    if (x < mouse->left)
      x = mouse->left;
    if (x > mouse->right)
      x = mouse->right;

    mouse->event.x = x;
    mouse->event.xrel = xrel;
    mouse->xctr = xctr;
  }

  yctr = joy0dat >> 8;
  yrel = yctr - mouse->yctr;

  if (yrel) {
    short y = mouse->event.y + yrel;

    if (y < mouse->top)
      y = mouse->top;
    if (y > mouse->bottom)
      y = mouse->bottom;

    mouse->event.y = y;
    mouse->event.yrel = yrel;
    mouse->yctr = yctr;
  }

  return xrel || yrel;
}

static inline u_char ReadButtonState(void) {
  u_char state = 0;

	if (!(ciaa->ciapra & CIAF_GAMEPORT0))
    state |= LMB_PRESSED;
  if (!(custom->potinp & DATLY))
    state |= RMB_PRESSED;

  return state;
}

static bool GetMouseButton(MouseDataT *mouse) {
  u_char button = ReadButtonState();
  u_char change = (mouse->button ^ button) & (LMB_PRESSED | RMB_PRESSED);

  if (!change)
    return false;

  mouse->button = button;

  if (change & LMB_PRESSED)
    mouse->event.button |= (button & LMB_PRESSED) ? LMB_PRESSED : LMB_RELEASED;

  if (change & RMB_PRESSED)
    mouse->event.button |= (button & RMB_PRESSED) ? RMB_PRESSED : RMB_RELEASED;

  return true;
}

static void MouseIntHandler(void *data) {
  MouseDataT *mouse = (MouseDataT *)data;

  mouse->event.button = 0;

  /* Register mouse position change first. */
  if (GetMouseMove(mouse))
    PushEventISR((EventT *)&mouse->event);

  /* After that a change in mouse button state. */
  if (GetMouseButton(mouse))
    PushEventISR((EventT *)&mouse->event);
}

INTSERVER(MouseServer, -5, (IntFuncT)MouseIntHandler, (void *)&MouseData);

void MouseInit(Box2D *win) {
  Log("[Mouse] Initialize driver!\n");

  /* Settings from MouseData structure. */
  MouseData.left = win->minX;
  MouseData.right = win->maxX;
  MouseData.top = win->minY;
  MouseData.bottom = win->maxY;
  MouseData.xctr = custom->joy0dat & 0xff;
  MouseData.yctr = custom->joy0dat >> 8;
  MouseData.button = ReadButtonState();
  MouseData.event.x = win->minX;
  MouseData.event.y = win->minY;
  MouseData.event.type = EV_MOUSE;

  AddIntServer(INTB_VERTB, MouseServer);
}

void MouseKill(void) {
  RemIntServer(INTB_VERTB, MouseServer);
}
