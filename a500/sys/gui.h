#ifndef __GUI_H__
#define __GUI_H__

#include "common.h"
#include "gfx.h"

#define EV_GUI 3

typedef enum {
  UI_BG_INACTIVE = 0,
  UI_BG_ACTIVE   = 1,
  UI_FRAME_IN    = 2,
  UI_FRAME_OUT   = 3,
  UI_FG_INACTIVE = 4,
  UI_FG_ACTIVE   = 5,
} GuiColorT;

typedef enum {
  FRAME_NONE, FRAME_FLAT, FRAME_IN, FRAME_OUT
} GuiFrameT;

typedef enum { 
  WT_LABEL, WT_BUTTON, WT_LAST
} WidgetTypeT;

typedef enum {
  WA_ENTERED, WA_LEFT, WA_PRESSED, WA_RELEASED, WA_LAST
} WidgetActionT;

typedef enum {
  WS_ACTIVE  = 1,
  WS_PRESSED = 2,
} WidgetStateT;

#define WIDGET_BASE             \
  UBYTE type;                   \
  /* widget definition */       \
  Area2D area;                  \
  /* widget internal state */   \
  WidgetStateT state;

typedef struct {
  WIDGET_BASE
} WidgetBaseT;

typedef struct {
  WIDGET_BASE
  char *text;
  WORD length;
  GuiFrameT frame;
} LabelT;

typedef struct {
  WIDGET_BASE
  char *label;
} ButtonT;

#undef WIDGET_BASE

typedef union Widget {
  UBYTE   type;
  LabelT  label;
  ButtonT button;
  WidgetBaseT base;
} WidgetT;

typedef struct GuiEvent {
  UBYTE type;
  UBYTE action;
  WidgetT *widget;
} GuiEventT;

typedef struct GuiState {
  WidgetT **widgets;
  WidgetBaseT *lastEntered;
  WidgetBaseT *lastPressed;
  BitmapT *screen;
  BitmapT *font;
} GuiStateT;

typedef void (*WidgetFuncT)(GuiStateT *, WidgetT *);

#define GUI_BUTTON(name, x, y, w, h, label)                              \
  WidgetT name[1] = {(WidgetT)(ButtonT)                                  \
    {WT_BUTTON, {(x), (y), (w), (h)}, 0, (label)}}

#define GUI_LABEL(name, x, y, w, h, frame, n)                            \
  WidgetT name[1] = {(WidgetT)(LabelT)                                   \
    {WT_LABEL, {(x), (y), (w), (h)}, 0, (char[(n)]){0}, (n), (frame)}}

#define GUI_GROUP(name, ...) \
  WidgetT *name[] = { __VA_ARGS__, NULL }

#define GUI_MAIN(widgets) \
  GuiStateT gui[1] = {{widgets, NULL}}

#define LabelFmtStr(wg, str, args...) \
  FmtStr(((LabelT *)(wg))->text, ((LabelT *)(wg))->length, (str), args)

struct MouseEvent;

void GuiInit(GuiStateT *gui, BitmapT *screen, BitmapT *font);
void GuiRedraw(GuiStateT *gui);
void GuiHandleMouseEvent(GuiStateT *gui, struct MouseEvent *ev);
void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg);

#endif
