#include <effect.h>
#include <copper.h>
#include <gui.h>
#include <sprite.h>
#include <stdio.h>
#include <system/event.h>
#include <system/keyboard.h>
#include <system/memory.h>
#include <system/mouse.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3

static BitmapT *screen;
static CopListT *cp;

#include "data/toggle_0.c"
#include "data/toggle_1.c"
#include "data/koi8r.8x8.c"
#include "data/pointer.c"

/* Test program */
static GUI_DEF(_b0, GUI_BUTTON(&GUI_LABEL("Play")));
static GUI_DEF(_b1, GUI_BUTTON(&GUI_LABEL("Pause")));
static GUI_DEF(_b2, GUI_BUTTON(&GUI_LABEL("Stop")));
static GUI_DEF(_b3, GUI_BUTTON(&GUI_LABEL("Forward")));
static GUI_DEF(_b4, GUI_BUTTON(&GUI_LABEL("Reverse")));
static GUI_DEF(_bg0, GUI_GROUP(5,
                               WG_ITEM(_b0, 0, 0, -1, -1),
                               WG_ITEM(_b1, 48, 20, -1, -1),
                               WG_ITEM(_b2, 96, 40, -1, -1),
                               WG_ITEM(_b3, 144, 60, -1, -1),
                               WG_ITEM(_b4, 192, 80, -1, -1)));

static GUI_DEF(_rb0, GUI_RADIOBT(&GUI_LABEL("A")));
static GUI_DEF(_rb1, GUI_RADIOBT(&GUI_LABEL("B")));
static GUI_DEF(_rb2, GUI_RADIOBT(&GUI_LABEL("C")));
static GUI_DEF(_rb3, GUI_RADIOBT(&GUI_LABEL("D")));
static GUI_DEF(_bg1, GUI_GROUP(4,
                               WG_ITEM(_rb0, 250, 14, -1, -1),
                               WG_ITEM(_rb1, 251, 28, -1, -1),
                               WG_ITEM(_rb2, 252, 42, -1, -1),
                               WG_ITEM(_rb3, 253, 56, -1, -1)));

static GUI_DEF(_t0, GUI_TOGGLE(&GUI_IMAGE(&toggle_0),
                               &GUI_IMAGE(&toggle_1)));
static GUI_DEF(_l0, GUI_LABEL_N(40));
static GUI_DEF(_f0, GUI_FRAME(FRAME_FLAT, _l0));
static GUI_DEF(_root, GUI_GROUP(4,
                                WG_ITEM(_bg0, 0, 0, -1, -1),
                                WG_ITEM(_bg1, 0, 0, -1, -1),
                                WG_ITEM(_f0, 0, 192, WIDTH, -1),
                                WG_ITEM(_t0, 160, 20, -1, -1)));
static GUI_MAIN(_root);

static void Load(void) {
  GuiInit(gui, &font);
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(120);
  CopInsPairT *sprptr = CopSetupSprites(cp);

  CopSetupBitplanes(cp, screen, DEPTH);
  CopInsSetSprite(&sprptr[0], &pointer);
  SpriteUpdatePos(&pointer, X(0), Y(0));
  return CopListFinish(cp);
}

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  SetColor(UI_BG_INACTIVE, 0x888);
  SetColor(UI_BG_ACTIVE, 0xaaa);
  SetColor(UI_FRAME_IN, 0x444);
  SetColor(UI_FRAME_OUT, 0xeee);
  SetColor(UI_FG_INACTIVE, 0x24a);
  SetColor(UI_FG_ACTIVE, 0x46e);
  LoadPalette(&pointer_pal, 16);

  cp = MakeCopperList();
  CopListActivate(cp);

  KeyboardInit();
  MouseInit(&(Box2D){.minX = 0, .minY = 0,
                     .maxX = WIDTH - 1, .maxY = HEIGHT - 1});

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);

  GuiRedraw(gui, screen);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);

  KeyboardKill();
  MouseKill();

  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static bool HandleEvent(void) {
  EventT ev[1];

  if (!PopEvent(ev))
    return true;

  if (ev->type == EV_KEY) {
   if (!(ev->key.modifier & MOD_PRESSED) && ev->key.code == KEY_ESCAPE)
      return false;
  } else if (ev->type == EV_MOUSE) {
    GuiHandleMouseEvent(gui, &ev->mouse);
    SpriteUpdatePos(&pointer, X(ev->mouse.x), Y(ev->mouse.y));
  } else if (ev->type == EV_GUI) {
    WidgetT *wg = ev->gui.widget;

    if (wg->type == WT_BUTTON || wg->type == WT_RADIOBT) {
      wg = wg->button.widget;

      if (wg->type == WT_LABEL) {
        LabelT *lb = &wg->label;
        static const char *action[] = {"Entered", "Left", "Pressed", "Released"};

        LabelFmtStr(_l0, "%s button '%s'!", action[ev->gui.action], lb->text);
        GuiWidgetRedraw(gui, _l0);
      }
    }
  }

  return true;
}

static void Render(void) {
  exitLoop = !HandleEvent();
}

EFFECT(GUI, Load, NULL, Init, Kill, Render, NULL);
