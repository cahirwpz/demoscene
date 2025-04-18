#include "debug.h"
#include "gui.h"
#include "gfx.h"
#include <system/mouse.h>
#include "blitter.h"

#define WidgetRedraw(gui, wg) \
  GuiWidgetRedraw((gui), (WidgetT *)wg)
#define WidgetEnter(gui, wg) \
  GuiWidgetEnter((gui), (WidgetT *)wg)
#define WidgetLeave(gui, wg) \
  GuiWidgetLeave((gui), (WidgetT *)wg)
#define WidgetPress(gui, wg) \
  GuiWidgetPress((gui), (WidgetT *)wg)
#define WidgetRelease(gui, wg) \
  GuiWidgetRelease((gui), (WidgetT *)wg)

static inline void GuiWidgetEnter(GuiStateT *gui, WidgetT *wg);
static inline void GuiWidgetLeave(GuiStateT *gui, WidgetT *wg);
static inline void GuiWidgetPress(GuiStateT *gui, WidgetT *wg);
static inline void GuiWidgetRelease(GuiStateT *gui, WidgetT *wg);

static inline void PushGuiEvent(WidgetT *wg, short event) {
  GuiEventT ev = { EV_GUI, event, wg };
  PushEvent((EventT *)&ev);
}

static void DrawFrame(BitmapT *bitmap, Area2D *area, GuiFrameT frame) {
  short x1 = area->x;
  short y1 = area->y;
  short x2 = area->x + area->w - 1;
  short y2 = area->y + area->h - 1;

  if (frame) {
    /* WC_FRAME_IN = 0b10 */
    BlitterLineSetup(bitmap, 1, LINE_OR|LINE_SOLID);
    BlitterLine(x1, y1, x2, y1);
    BlitterLine(x1, y2, x2, y2);
    BlitterLine(x1, y1 + 1, x1, y2 - 1);
    BlitterLine(x2, y1 + 1, x2, y2 - 1);

    /* WC_FRAME_OUT = 0b11 */
    BlitterLineSetup(bitmap, 0, LINE_EOR|LINE_SOLID);

    if (frame == FRAME_IN) {
      BlitterLine(x2, y1 + 1, x2, y2);
      BlitterLine(x1 + 1, y2, x2 - 1, y2);
    } else if (frame == FRAME_OUT) {
      BlitterLine(x1, y1, x2, y1);
      BlitterLine(x1, y1 + 1, x1, y2);
    }
  }
}

static void GroupRedraw(GuiStateT *gui, GroupT *wg) {
  GroupItemT *item;
  short n;

  for (item = wg->item, n = wg->count; n--; item++)
    WidgetRedraw(gui, item->widget);
}

static void LabelRedraw(GuiStateT *gui, LabelT *wg) {
  FontDrawCtxT ctx = { gui->font, gui->screen, &wg->area, 2 };
  bool active = (wg->parent->base.state & WS_ACTIVE) ? true : false;

  BitmapSetArea(gui->screen, &wg->area, active ? UI_BG_ACTIVE : UI_BG_INACTIVE);
  DrawText(&ctx, wg->text);
}

static void ImageRedraw(GuiStateT *gui, ImageT *wg) {
  BlitterCopy(gui->screen, 2, wg->area.x, wg->area.y, wg->bm, 0);
}

static void FrameRedraw(GuiStateT *gui, FrameT *wg) {
  BitmapSetArea(gui->screen, &wg->area, UI_BG_INACTIVE);
  DrawFrame(gui->screen, &wg->area, 1);

  if (wg->widget)
    WidgetRedraw(gui, wg->widget);
}

static void ButtonRedraw(GuiStateT *gui, ButtonT *wg) {
  bool active = (wg->state & WS_ACTIVE) ? 1 : 0;
  bool pressed = (wg->state & WS_PRESSED) ? 1 : 0;
  bool toggled = (wg->state & WS_TOGGLED) ? 1 : 0;

  BitmapSetArea(gui->screen, &wg->area,
                active ? UI_BG_ACTIVE : UI_BG_INACTIVE);
  DrawFrame(gui->screen, &wg->area,
            (active ^ pressed ^ toggled) ? FRAME_IN : FRAME_OUT);

  if (wg->widget)
    WidgetRedraw(gui, wg->widget);
}

static void ToggleRedraw(GuiStateT *gui, ToggleT *wg) {
  bool active = (wg->state & WS_ACTIVE) ? 1 : 0;
  bool pressed = (wg->state & WS_PRESSED) ? 1 : 0;
  bool toggled = (wg->state & WS_TOGGLED) ? 1 : 0;

  BitmapSetArea(gui->screen, &wg->area,
                active ? UI_BG_ACTIVE : UI_BG_INACTIVE);
  DrawFrame(gui->screen, &wg->area,
            (active ^ pressed) ? FRAME_IN : FRAME_OUT);

  if (wg->widget[toggled])
    WidgetRedraw(gui, wg->widget[toggled]);
}

static void ButtonPress(GuiStateT *gui, ButtonT *wg) {
  wg->state |= WS_PRESSED;
  PushGuiEvent((WidgetT *)wg, WA_PRESSED);
  WidgetRedraw(gui, wg);
}

static void ButtonRelease(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_PRESSED;
  if (gui->lastPressed == (WidgetBaseT *)wg)
    PushGuiEvent((WidgetT *)wg, WA_RELEASED);
  WidgetRedraw(gui, wg);
}

static void RadioButtonRelease(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_PRESSED;
  if (gui->lastPressed == (WidgetBaseT *)wg) {
    PushGuiEvent((WidgetT *)wg, WA_RELEASED);
    if (!(wg->state & WS_TOGGLED)) {
      GroupT *group = &wg->parent->group;
      short i;

      for (i = 0; i < group->count; i++) {
        WidgetT *child = group->item[i].widget;
        if (child->base.state & WS_TOGGLED) {
          child->base.state &= ~WS_TOGGLED;
          WidgetRedraw(gui, child);
        }
      }

      wg->state |= WS_TOGGLED;
    }
  }
  WidgetRedraw(gui, wg);
}

static void ToggleRelease(GuiStateT *gui, ToggleT *wg) {
  wg->state &= ~WS_PRESSED;
  if (gui->lastPressed == (WidgetBaseT *)wg) {
    wg->state ^= WS_TOGGLED;
    PushGuiEvent((WidgetT *)wg, WA_RELEASED);
  }
  WidgetRedraw(gui, wg);
}

static void ButtonLeave(GuiStateT *gui, ButtonT *wg) {
  wg->state &= ~WS_ACTIVE;
  wg->state &= ~WS_PRESSED;
  PushGuiEvent((WidgetT *)wg, WA_LEFT);
  WidgetRedraw(gui, wg);
}

static void ButtonEnter(GuiStateT *gui, ButtonT *wg) {
  wg->state |= WS_ACTIVE;
  if (gui->lastPressed == (WidgetBaseT *)wg)
    wg->state |= WS_PRESSED;
  PushGuiEvent((WidgetT *)wg, WA_ENTERED);
  WidgetRedraw(gui, wg);
}

/* Dynamic function dispatch. */
static void WidgetDummyFunc(GuiStateT *gui __unused, ButtonT *wg __unused) {}

void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetRedrawFunc[WT_LAST] = {
    [WT_GROUP] = (WidgetFuncT)GroupRedraw,
    [WT_FRAME] = (WidgetFuncT)FrameRedraw,
    [WT_LABEL] = (WidgetFuncT)LabelRedraw,
    [WT_IMAGE] = (WidgetFuncT)ImageRedraw,
    [WT_BUTTON] = (WidgetFuncT)ButtonRedraw,
    [WT_RADIOBT] = (WidgetFuncT)ButtonRedraw,
    [WT_TOGGLE] = (WidgetFuncT)ToggleRedraw,
  };
  WidgetRedrawFunc[(wg)->type](gui, wg);
}

static inline void GuiWidgetEnter(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetEnterFunc[WT_LAST] = {
    [WT_GROUP] = (WidgetFuncT)WidgetDummyFunc,
    [WT_FRAME] = (WidgetFuncT)WidgetDummyFunc,
    [WT_LABEL] = (WidgetFuncT)WidgetDummyFunc,
    [WT_IMAGE] = (WidgetFuncT)WidgetDummyFunc,
    [WT_BUTTON] = (WidgetFuncT)ButtonEnter,
    [WT_RADIOBT] = (WidgetFuncT)ButtonEnter,
    [WT_TOGGLE] = (WidgetFuncT)ButtonEnter,
  };
  WidgetEnterFunc[(wg)->type]((gui), wg);
}

static inline void GuiWidgetLeave(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetLeaveFunc[WT_LAST] = {
    [WT_GROUP] = (WidgetFuncT)WidgetDummyFunc,
    [WT_FRAME] = (WidgetFuncT)WidgetDummyFunc,
    [WT_LABEL] = (WidgetFuncT)WidgetDummyFunc,
    [WT_IMAGE] = (WidgetFuncT)WidgetDummyFunc,
    [WT_BUTTON] = (WidgetFuncT)ButtonLeave,
    [WT_RADIOBT] = (WidgetFuncT)ButtonLeave,
    [WT_TOGGLE] = (WidgetFuncT)ButtonLeave,
  };
  WidgetLeaveFunc[(wg)->type]((gui), wg);
}

static inline void GuiWidgetPress(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetPressFunc[WT_LAST] = {
    [WT_GROUP] = (WidgetFuncT)WidgetDummyFunc,
    [WT_FRAME] = (WidgetFuncT)WidgetDummyFunc,
    [WT_LABEL] = (WidgetFuncT)WidgetDummyFunc,
    [WT_IMAGE] = (WidgetFuncT)WidgetDummyFunc,
    [WT_BUTTON] = (WidgetFuncT)ButtonPress,
    [WT_RADIOBT] = (WidgetFuncT)ButtonPress,
    [WT_TOGGLE] = (WidgetFuncT)ButtonPress,
  };
  WidgetPressFunc[(wg)->type]((gui), wg);
}

static inline void GuiWidgetRelease(GuiStateT *gui, WidgetT *wg) {
  static WidgetFuncT WidgetReleaseFunc[WT_LAST] = {
    [WT_GROUP] = (WidgetFuncT)WidgetDummyFunc,
    [WT_FRAME] = (WidgetFuncT)WidgetDummyFunc,
    [WT_LABEL] = (WidgetFuncT)WidgetDummyFunc,
    [WT_IMAGE] = (WidgetFuncT)WidgetDummyFunc,
    [WT_BUTTON] = (WidgetFuncT)ButtonRelease,
    [WT_RADIOBT] = (WidgetFuncT)RadioButtonRelease,
    [WT_TOGGLE] = (WidgetFuncT)ToggleRelease,
  };
  WidgetReleaseFunc[(wg)->type]((gui), wg);
}

static WidgetT *FindWidgetByMouse(WidgetT *wg, short x, short y) {
  if (wg->type == WT_GROUP) {
    GroupT *group = &wg->group;
    short i;

    for (i = 0; i < group->count; i++)
      if ((wg = FindWidgetByMouse(group->item[i].widget, x, y)))
        return wg;

    return NULL;
  }

  return InsideArea(x, y, &wg->base.area) ? wg : NULL;
}

#define POS_X(wg) ((wg)->base.area.x)
#define POS_Y(wg) ((wg)->base.area.y)
#define WIDTH(wg) ((wg)->base.area.w)
#define HEIGHT(wg) ((wg)->base.area.h)

/* Initialize pointers to a parent. Load resources. */
static void WidgetInit(GuiStateT *gui, WidgetT *wg, WidgetT *parent) {
  wg->base.parent = parent;

  if (wg->type == WT_GROUP) {
    short i;
    for (i = 0; i < wg->group.count; i++)
      WidgetInit(gui, wg->group.item[i].widget, wg);
  } else if (wg->type == WT_BUTTON || wg->type == WT_RADIOBT) {
    WidgetInit(gui, wg->button.widget, wg);
  } else if (wg->type == WT_TOGGLE) {
    WidgetInit(gui, wg->toggle.widget[0], wg);
    WidgetInit(gui, wg->toggle.widget[1], wg);
  } else if (wg->type == WT_FRAME) {
    WidgetInit(gui, wg->frame.widget, wg);
  } else if (wg->type == WT_IMAGE) {
    if (!wg->image.bm)
      Panic("No image attached to widget at %p!", wg);
  }
}

/* Calculates widget position in top-down approach. */
static void WidgetCalcPos(GuiStateT *gui, WidgetT *wg, short x, short y) {
  if (wg->type == WT_GROUP) {
    GroupT *group = &wg->group;
    short i;

    for (i = 0; i < group->count; i++) {
      GroupItemT *item = &group->item[i];
      WidgetCalcPos(gui, item->widget, item->req.x, item->req.y);
    }
  } else {
    POS_X(wg) = x, POS_Y(wg) = y;

    if (wg->type == WT_BUTTON || wg->type == WT_RADIOBT) {
      WidgetCalcPos(gui, wg->button.widget, x + 2, y + 2);
    } else if (wg->type == WT_TOGGLE) {
      WidgetCalcPos(gui, wg->toggle.widget[0], x + 2, y + 2);
      WidgetCalcPos(gui, wg->toggle.widget[1], x + 2, y + 2);
    } else if (wg->type == WT_FRAME) {
      WidgetCalcPos(gui, wg->frame.widget, x + 2, y + 2);
    }
  }
}

static void WidgetCalcSize(GuiStateT *gui, WidgetT *wg, short w, short h) {
  if (wg->type == WT_GROUP) {
    GroupT *group = &wg->group;
    short i, nw = 0, nh = 0;

    /* Calculate largest box containing all children.
     * Make children calculate their requested size. */
    for (i = 0; i < group->count; i++) {
      GroupItemT *item = &group->item[i];

      WidgetCalcSize(gui, item->widget, item->req.w, item->req.h);
      if (WIDTH(item->widget) > nw)
        nw = WIDTH(item->widget);
      if (HEIGHT(item->widget) > nh)
        nh = HEIGHT(item->widget);
    }

    /* If user didn't specify group size use calculated one. */
    WIDTH(wg) = max(nw, w);
    HEIGHT(wg) = max(nh, h);
  } else if (wg->type == WT_BUTTON || wg->type == WT_RADIOBT) {
    WidgetCalcSize(gui, wg->button.widget, w - 4, h - 4);
    WIDTH(wg) = max(w, WIDTH(wg->button.widget) + 4);
    HEIGHT(wg) = max(h, HEIGHT(wg->button.widget) + 4);
  } else if (wg->type == WT_TOGGLE) {
    WidgetT *w_off = wg->toggle.widget[0];
    WidgetT *w_on = wg->toggle.widget[1];
    short nw, nh;
    WidgetCalcSize(gui, w_off, w - 4, h - 4);
    WidgetCalcSize(gui, w_on, w - 4, h - 4);
    nw = max(WIDTH(w_off), WIDTH(w_on));
    nh = max(HEIGHT(w_off), HEIGHT(w_on));
    WIDTH(wg) = max(w, nw + 4);
    HEIGHT(wg) = max(h, nh + 4);
  } else if (wg->type == WT_FRAME) {
    WidgetCalcSize(gui, wg->frame.widget, w - 4, h - 4);
    WIDTH(wg) = max(w, WIDTH(wg->frame.widget) + 4);
    HEIGHT(wg) = max(h, HEIGHT(wg->frame.widget) + 4);
  } else if (wg->type == WT_LABEL) {
    Size2D size = DrawTextSizeN(gui->font, wg->label.text, wg->label.length);
    WIDTH(wg) = max(w, size.w);
    HEIGHT(wg) = max(h, size.h);
  } else if (wg->type == WT_IMAGE) {
    if (!wg->image.bm)
      Panic("No image attached to widget at %p!", wg);
    WIDTH(wg) = max(w, wg->image.bm->width);
    HEIGHT(wg) = max(h, wg->image.bm->height);
  }
}

void GuiInit(GuiStateT *gui, FontT *font) {
  gui->font = font;
  WidgetInit(gui, gui->root, NULL);
}

void GuiRedraw(GuiStateT *gui, BitmapT *screen) {
  gui->screen = screen;

  WidgetCalcPos(gui, gui->root, 0, 0);
  WidgetCalcSize(gui, gui->root, screen->width, screen->height);
  WidgetRedraw(gui, gui->root);
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
