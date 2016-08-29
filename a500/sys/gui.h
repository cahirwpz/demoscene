#ifndef __GUI_H__
#define __GUI_H__

#include "common.h"
#include "event.h"
#include "gfx.h"
#include "font.h"

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
  WT_GROUP, WT_FRAME, WT_LABEL, WT_IMAGE, WT_BUTTON, WT_RADIOBT, WT_LAST
} WidgetTypeT;

typedef enum {
  WA_ENTERED, WA_LEFT, WA_PRESSED, WA_RELEASED, WA_LAST
} WidgetActionT;

typedef enum {
  WS_ACTIVE  = 1,
  WS_PRESSED = 2,
  WS_TOGGLED = 4,
} WidgetStateT;

typedef union Widget WidgetT;

#define WIDGET_BASE                                                     \
  UBYTE type;                                                           \
  /* widget absolute position and size */                               \
  Area2D area;                                                          \
  /* widget internal state */                                           \
  WidgetStateT state;                                                   \
  /* pointer to parent */                                               \
  WidgetT *parent;

typedef struct {
  WIDGET_BASE
} WidgetBaseT;

typedef struct {
  WidgetT *widget;
  Area2D req;
} GroupItemT;

typedef struct {
  WIDGET_BASE
  WORD count;
  GroupItemT *item;
} GroupT;

typedef struct {
  WIDGET_BASE
  GuiFrameT frame;
  WidgetT *widget;
} FrameT;

typedef struct {
  WIDGET_BASE
  char *text;
  WORD length;
} LabelT;

typedef struct {
  WIDGET_BASE
  char *path;
  BitmapT *bm;
} ImageT;

typedef struct {
  WIDGET_BASE
  WidgetT *widget;
} ButtonT;

#undef WIDGET_BASE

union Widget {
  UBYTE   type;
  GroupT  group;
  FrameT  frame;
  ImageT  image;
  LabelT  label;
  ButtonT button;
  WidgetBaseT base;
};

typedef struct GuiState {
  WidgetT *root;
  WidgetBaseT *lastEntered;
  WidgetBaseT *lastPressed;
  BitmapT *screen;
  FontT *font;
} GuiStateT;

typedef void (*WidgetFuncT)(GuiStateT *, WidgetT *);

#define WG_ITEM(widget, x, y, w, h) \
  ((GroupItemT){(widget), (Area2D){(x), (y), (w), (h)}})

#define GUI_DEF(name, widget)                                           \
  WidgetT name[1] = widget

#define GUI_WIDGET(ctype, wtype, ...)                                   \
  (WidgetT[1]){(WidgetT)(ctype){(wtype), (Area2D){0}, 0, NULL, ##__VA_ARGS__}}

#define GUI_FRAME(frame, widget)                                        \
  GUI_WIDGET(FrameT, WT_FRAME, (frame), (widget))

#define GUI_LABEL(text)                                                 \
  GUI_WIDGET(LabelT, WT_LABEL, text, sizeof(text) + 1)

#define GUI_LABEL_N(size)                                               \
  GUI_WIDGET(LabelT, WT_LABEL, (char[(size)]){0}, (size))

#define GUI_IMAGE(path)                                                 \
  GUI_WIDGET(ImageT, WT_IMAGE, (path), NULL)

#define GUI_BUTTON(inner)                                               \
  GUI_WIDGET(ButtonT, WT_BUTTON, (inner))

#define GUI_RADIOBT(inner)                                              \
  GUI_WIDGET(ButtonT, WT_RADIOBT, (inner))

#define GUI_GROUP_ITEMS(count, ...)                                     \
  count, (GroupItemT[count]){##__VA_ARGS__}

#define GUI_GROUP(...)                                                  \
  GUI_WIDGET(GroupT, WT_GROUP,                                          \
             GUI_GROUP_ITEMS(VA_NARGS(__VA_ARGS__), __VA_ARGS__))

#define GUI_MAIN(root) \
  GuiStateT gui[1] = {{root}}

#define LabelFmtStr(wg, str, ...) \
  FmtStr(((LabelT *)(wg))->text, ((LabelT *)(wg))->length, (str), __VA_ARGS__)

void GuiInit(GuiStateT *gui, FontT *font);
void GuiRedraw(GuiStateT *gui, BitmapT *screen);
void GuiHandleMouseEvent(GuiStateT *gui, MouseEventT *ev);
void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg);

#endif
