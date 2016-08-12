#include "gui.h"
#include "gfx.h"
#include "mouse.h"
#include "bltop.h"

static inline void PushGuiEvent(WidgetT *wg, WORD event) {
  GuiEventT ev = { EV_GUI, event, wg };
  PushEvent((EventT *)&ev);
}

static void DrawFrame(BitmapT *bitmap, Area2D *area, GuiFrameT frame) {
  WORD x1 = area->x;
  WORD y1 = area->y;
  WORD x2 = area->x + area->w - 1;
  WORD y2 = area->y + area->h - 1;

  if (frame) {
    /* WC_FRAME_IN = 0b10 */
    BlitterLineSetup(bitmap, 1, LINE_OR, LINE_SOLID);
    BlitterLine(x1, y1, x2, y1);
    BlitterLine(x1, y2, x2, y2);
    BlitterLine(x1, y1 + 1, x1, y2 - 1);
    BlitterLine(x2, y1 + 1, x2, y2 - 1);

    /* WC_FRAME_OUT = 0b11 */
    BlitterLineSetup(bitmap, 0, LINE_EOR, LINE_SOLID);

    if (frame == FRAME_IN) {
      BlitterLine(x2, y1 + 1, x2, y2);
      BlitterLine(x1 + 1, y2, x2 - 1, y2);
    } else if (frame == FRAME_OUT) {
      BlitterLine(x1, y1, x2, y1);
      BlitterLine(x1, y1 + 1, x1, y2);
    }
  }
}

inline void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg);

static void GroupRedraw(GuiStateT *gui, GroupT *wg) {
  WidgetT **widgets = wg->widgets;
  WidgetT *w;

  while ((w = *widgets++))
    GuiWidgetRedraw(gui, w);
}

static void LabelRedraw(GuiStateT *gui, LabelT *wg) {
  BitmapSetArea(gui->screen, &wg->area, UI_BG_INACTIVE);

  if (wg->frame) {
    DrawFrame(gui->screen, &wg->area, 1);
    DrawText(wg->area.x + 2, wg->area.y + 2, wg->text);
  } else {
    DrawText(wg->area.x, wg->area.y, wg->text);
  }
}

static void ButtonRedraw(GuiStateT *gui, ButtonT *wg) {
  BOOL active = (wg->state & WS_ACTIVE) ? 1 : 0;
  BOOL pressed = (wg->state & WS_PRESSED) ? 1 : 0;
  BOOL toggled = (wg->state & WS_TOGGLED) ? 1 : 0;

  BitmapSetArea(gui->screen, &wg->area,
                active ? UI_BG_ACTIVE : UI_BG_INACTIVE);
  DrawFrame(gui->screen, &wg->area,
            (active ^ pressed ^ toggled) ? FRAME_IN : FRAME_OUT);

  if (wg->label)
    DrawText(wg->area.x + 2, wg->area.y + 2, wg->label);
}

static void ButtonPress(GuiStateT *gui, ButtonT *wg) {
  wg->state |= WS_PRESSED;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_PRESSED);
}

static void ButtonRelease(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_PRESSED;
  ButtonRedraw(gui, wg);
  if (gui->lastPressed == (WidgetBaseT *)wg)
    PushGuiEvent((WidgetT *)wg, WA_RELEASED);
}

static void RadioButtonRelease(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_PRESSED;
  if (gui->lastPressed == (WidgetBaseT *)wg) {
    PushGuiEvent((WidgetT *)wg, WA_RELEASED);
    if (!(wg->state & WS_TOGGLED)) {
      WidgetT **widgets = wg->parent->group.widgets;
      WidgetT *child;

      while ((child = *widgets++)) {
        if (child->base.state & WS_TOGGLED) {
          child->base.state &= ~WS_TOGGLED;
          GuiWidgetRedraw(gui, child);
        }
      }

      wg->state |= WS_TOGGLED;
    }
  }
  ButtonRedraw(gui, wg);
}

static void ButtonLeave(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_ACTIVE;
  wg->state &= ~WS_PRESSED;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_LEFT);
}

static void ButtonEnter(GuiStateT *gui, ButtonT *wg) {
  wg->state |= WS_ACTIVE;
  if (gui->lastPressed == (WidgetBaseT *)wg)
    wg->state |= WS_PRESSED;
  ButtonRedraw(gui, wg);
  PushGuiEvent((WidgetT *)wg, WA_ENTERED);
}

/* Dynamic function dispatch. */
static void WidgetDummyFunc(GuiStateT *gui, ButtonT *wg) {}

inline void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetRedrawFunc[WT_LAST] = {
    (WidgetFuncT)GroupRedraw, 
    (WidgetFuncT)LabelRedraw, 
    (WidgetFuncT)ButtonRedraw,
    (WidgetFuncT)ButtonRedraw,
  };
  WidgetRedrawFunc[(wg)->type](gui, (WidgetT *)wg);
}

static WidgetFuncT WidgetEnterFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonEnter,
  (WidgetFuncT)ButtonEnter,
};

#define WidgetEnter(gui, wg) \
  WidgetEnterFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetFuncT WidgetLeaveFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonLeave,
  (WidgetFuncT)ButtonLeave,
};

#define WidgetLeave(gui, wg) \
  WidgetLeaveFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetFuncT WidgetPressFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonPress,
  (WidgetFuncT)ButtonPress,
};

#define WidgetPress(gui, wg) \
  WidgetPressFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetFuncT WidgetReleaseFunc[WT_LAST] = {
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)WidgetDummyFunc,
  (WidgetFuncT)ButtonRelease,
  (WidgetFuncT)RadioButtonRelease,
};

#define WidgetRelease(gui, wg) \
  WidgetReleaseFunc[(wg)->type]((gui), (WidgetT *)wg)

static WidgetT *FindWidgetByMouse(WidgetT *wg, WORD x, WORD y) {
  if (wg->type == WT_GROUP) {
    WidgetT **widgets = wg->group.widgets;
    WidgetT *child;

    while ((child = *widgets++))
      if ((wg = FindWidgetByMouse(child, x, y)))
        return wg;

    return NULL;
  }

  return InsideArea(x, y, &wg->base.area) ? wg : NULL;
}

static void InitWidget(WidgetT *wg, WidgetT *parent) {
  if (wg->type == WT_GROUP) {
    WidgetT **widgets = wg->group.widgets;
    WidgetT *child;

    while ((child = *widgets++))
      InitWidget(child, wg);
  } else {
    wg->base.parent = parent;
  }
}

void GuiInit(GuiStateT *gui, BitmapT *screen, FontT *font) {
  gui->screen = screen;
  gui->font = font;

  InitWidget(gui->root, NULL);
  DrawTextSetup(gui->screen, 2, gui->font);
}

void GuiRedraw(GuiStateT *gui) {
  GuiWidgetRedraw(gui, gui->root);
}

void GuiHandleMouseEvent(GuiStateT *gui, struct MouseEvent *mouse) {
  WidgetBaseT *wg = gui->lastEntered;

  if (wg) {
    if (InsideArea(mouse->x, mouse->y, &wg->area)) {
      if (mouse->button & LMB_PRESSED) {
        WidgetPress(gui, wg);
        gui->lastPressed = wg;
      } else if (mouse->button & LMB_RELEASED) {
        WidgetRelease(gui, wg);
        gui->lastPressed = NULL;
      }
      return;
    }
    WidgetLeave(gui, wg);
  }

  if (mouse->button & LMB_RELEASED)
    gui->lastPressed = NULL;
  
  /* Find the widget a pointer is hovering on. */
  wg = (WidgetBaseT *)FindWidgetByMouse(gui->root, mouse->x, mouse->y);

  gui->lastEntered = wg;

  if (wg)
    WidgetEnter(gui, wg);
}
