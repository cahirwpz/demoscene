#ifndef __GUI_H__
#define __GUI_H__

#include "common.h"
#include "gfx.h"
#include "font.h"
#include <system/event.h>

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
  WT_GROUP, WT_FRAME, WT_LABEL, WT_IMAGE, WT_BUTTON, WT_RADIOBT, WT_TOGGLE,
  WT_LAST
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
  u_char type;                                                           \
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
  short count;
  GroupItemT *item;
} GroupT;

typedef struct {
  WIDGET_BASE
  GuiFrameT frame;
  WidgetT *widget;
} FrameT;

typedef struct {
  WIDGET_BASE
  const char *text;
  short length;
} LabelT;

typedef struct {
  WIDGET_BASE
  const BitmapT *bm;
} ImageT;

typedef struct {
  WIDGET_BASE
  WidgetT *widget;
} ButtonT;

typedef struct {
  WIDGET_BASE
  WidgetT *widget[2];
} ToggleT;

#undef WIDGET_BASE

union Widget {
  u_char  type;
  GroupT  group;
  FrameT  frame;
  ImageT  image;
  LabelT  label;
  ButtonT button;
  ToggleT toggle;
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

#define WG_ITEM(wdg, x, y, w, h)                                        \
  {.widget = (wdg), .req = {(x), (y), (w), (h)}}

#define GUI_DEF(name, wdg)                                              \
  WidgetT name[1] = {wdg}

#define GUI_WIDGET(variant, wtype, ...)                                 \
  (WidgetT){.variant = {                                               \
    .type = (wtype), .area = {0, 0, 0, 0}, .state = 0, .parent = NULL,  \
    __VA_ARGS__}}

#define GUI_FRAME(frm, wdg) GUI_WIDGET(frame, WT_FRAME, (frm), (wdg))
#define GUI_LABEL(text) GUI_WIDGET(label, WT_LABEL, text, sizeof(text) + 1)
#define GUI_LABEL_N(size) GUI_WIDGET(label, WT_LABEL, (char[(size)]){0}, (size))
#define GUI_IMAGE(bm) GUI_WIDGET(image, WT_IMAGE, (bm))
#define GUI_BUTTON(inner) GUI_WIDGET(button, WT_BUTTON, (inner))
#define GUI_RADIOBT(inner) GUI_WIDGET(button, WT_RADIOBT, (inner))
#define GUI_TOGGLE(off, on) GUI_WIDGET(toggle, WT_TOGGLE, {(off), (on)})

#define GUI_GROUP(cnt, ...)                                             \
  GUI_WIDGET(group, WT_GROUP,                                           \
             .count = (cnt), .item = (GroupItemT[cnt]){__VA_ARGS__})

#define GUI_MAIN(root)                                                  \
  GuiStateT *gui = &(GuiStateT){root, NULL, NULL, NULL, NULL}

#define LabelFmtStr(wg, str, ...)                                       \
  snprintf(__DECONST(char *, ((LabelT *)(wg))->text),                   \
           ((LabelT *)(wg))->length, (str), __VA_ARGS__)

void GuiInit(GuiStateT *gui, FontT *font);
void GuiRedraw(GuiStateT *gui, BitmapT *screen);
void GuiHandleMouseEvent(GuiStateT *gui, MouseEventT *ev);
void GuiWidgetRedraw(GuiStateT *gui, WidgetT *wg);

#endif
