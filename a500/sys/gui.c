#include "gui.h"
#include "gfx.h"
#include "mouse.h"
#include "event.h"
#include "blitter.h"

static inline void PushGuiEvent(WidgetT *wg, WORD event) {
  GuiEventT ev = { EV_GUI, event, wg };
  PushEvent((EventT *)&ev);
}

/* Widget handling functions. */
static void DrawText(GuiStateT *gui, WORD x, WORD y, UBYTE *text) {
  WORD swidth = gui->font->bytesPerRow;
  WORD dwidth = gui->screen->bytesPerRow;
  UBYTE *src = gui->font->planes[0];
  UBYTE *dst = gui->screen->planes[2] + y * dwidth;
  WORD n = strlen(text);
  WORD k = (x + 7) >> 3;

  while (--n >= 0) {
    WORD i = *text++;
    WORD j = k++;
    WORD h = 8;

    if (i == '\n') {
      dst += dwidth * 8;
      k = x >> 3;
    }

    i -= 32;

    if (i < 0)
      continue;

    while (--h >= 0) {
      dst[j] |= src[i];
      i += swidth;
      j += dwidth;
    }
  }
}

static void LabelRedraw(GuiStateT *gui, LabelT *wg) {
  BlitterSetArea(gui->screen, 2, &wg->area, 0);
  DrawText(gui, wg->area.x, wg->area.y, wg->text);
}

static void ButtonRedraw(GuiStateT *gui, ButtonT *wg) {
  WORD x1 = wg->area.x;
  WORD y1 = wg->area.y;
  WORD x2 = wg->area.x + wg->area.w - 1;
  WORD y2 = wg->area.y + wg->area.h - 1;
  BOOL active = (wg->state & WS_ACTIVE) ? 1 : 0;
  BOOL pressed = (wg->state & WS_PRESSED) ? 1 : 0;

  BlitterSetArea(gui->screen, 0, &wg->area, active ? -1 : 0);
  BlitterSetArea(gui->screen, 1, &wg->area, 0);

  /* WC_FRAME_IN = 0b10 */
  BlitterLineSetup(gui->screen, 1, LINE_OR, LINE_SOLID);
  BlitterLine(x1, y1, x2, y1);
  BlitterLine(x1, y2, x2, y2);
  BlitterLine(x1, y1 + 1, x1, y2 - 1);
  BlitterLine(x2, y1 + 1, x2, y2 - 1);

  /* WC_FRAME_OUT = 0b11 */
  BlitterLineSetup(gui->screen, 0, LINE_EOR, LINE_SOLID);

  if (active ^ pressed) {
    BlitterLine(x2, y1 + 1, x2, y2);
    BlitterLine(x1 + 1, y2, x2 - 1, y2);
  } else {
    BlitterLine(x1, y1, x2, y1);
    BlitterLine(x1, y1 + 1, x1, y2);
  }

  if (wg->label) {
    BlitterSetArea(gui->screen, 2, &wg->area, 0);
    DrawText(gui, x1 + 1, y1 + 1, wg->label);
  }
}

static void ButtonPress(GuiStateT *gui, ButtonT *wg) {
  wg->state |= WS_PRESSED;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_PRESSED);
}

static void ButtonRelease(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_PRESSED;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_RELEASED);
}

static void ButtonLeave(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_ACTIVE;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_LEFT);
}

static void ButtonEnter(GuiStateT *gui, ButtonT *wg) {
  wg->state |= WS_ACTIVE;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_ENTERED);
}

/* Dynamic function dispatch. */
static void WidgetDummyFunc(GuiStateT *gui, ButtonT *wg) {}

inline void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetRedrawFunc[WT_LAST] = {
    (WidgetFuncT)LabelRedraw, 
    (WidgetFuncT)ButtonRedraw,
  };
  WidgetRedrawFunc[(wg)->type](gui, (WidgetT *)wg);
}

static WidgetFuncT WidgetEnterFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonEnter,
};

#define WidgetEnter(gui, wg) \
  WidgetEnterFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetFuncT WidgetLeaveFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonLeave,
};

#define WidgetLeave(gui, wg) \
  WidgetLeaveFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetFuncT WidgetPressFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonPress,
};

#define WidgetPress(gui, wg) \
  WidgetPressFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetFuncT WidgetReleaseFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonRelease,
};

#define WidgetRelease(gui, wg) \
  WidgetReleaseFunc[(wg)->type]((gui), (WidgetT *)wg)

void GuiInit(GuiStateT *gui, BitmapT *screen, BitmapT *font) {
  gui->screen = screen;
  gui->font = font;
}

void GuiRedraw(GuiStateT *gui) {
  WidgetT **widgets = gui->widgets;
  WidgetT *wg;
  
  while ((wg = *widgets++))
    GuiWidgetRedraw(gui, wg);
}

void GuiHandleMouseEvent(GuiStateT *gui, struct MouseEvent *mouse) {
  WidgetBaseT *wg = gui->lastWidget;

  if (wg) {
    if (InsideArea(mouse->x, mouse->y, &wg->area)) {
      if (mouse->button & LMB_PRESSED)
        WidgetPress(gui, wg);
      else if (mouse->button & LMB_RELEASED)
        WidgetRelease(gui, wg);
      return;
    } else {
      WidgetLeave(gui, wg);
    }
  }
  
  /* Find the widget a pointer is hovering on. */
  {
    WidgetBaseT **widgets = (WidgetBaseT **)gui->widgets;

    while ((wg = *widgets++))
      if (InsideArea(mouse->x, mouse->y, &wg->area))
        break;
  }

  gui->lastWidget = wg;

  if (wg)
    WidgetEnter(gui, wg);
}
