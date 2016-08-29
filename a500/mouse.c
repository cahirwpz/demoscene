#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "sprite.h"
#include "gui.h"
#include "event.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"
#include "ilbm.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3

STRPTR __cwdpath = "data";

static BitmapT *screen;
static CopListT *cp;
static SpriteT *pointer;
static CopInsT *sprptr[8];
static FontT *font;

/* Test program */
static GUI_DEF(_b0, GUI_BUTTON(GUI_LABEL("Play")));
static GUI_DEF(_b1, GUI_BUTTON(GUI_LABEL("Pause")));
static GUI_DEF(_b2, GUI_BUTTON(GUI_LABEL("Stop")));
static GUI_DEF(_b3, GUI_BUTTON(GUI_LABEL("Forward")));
static GUI_DEF(_b4, GUI_BUTTON(GUI_LABEL("Reverse")));
static GUI_DEF(_bg0, GUI_GROUP(WG_ITEM(_b0, 0, 0, -1, -1),
                               WG_ITEM(_b1, 48, 20, -1, -1),
                               WG_ITEM(_b2, 96, 40, -1, -1),
                               WG_ITEM(_b3, 144, 60, -1, -1),
                               WG_ITEM(_b4, 192, 80, -1, -1)));

static GUI_DEF(_rb0, GUI_RADIOBT(GUI_LABEL("A")));
static GUI_DEF(_rb1, GUI_RADIOBT(GUI_LABEL("B")));
static GUI_DEF(_rb2, GUI_RADIOBT(GUI_LABEL("C")));
static GUI_DEF(_rb3, GUI_RADIOBT(GUI_LABEL("D")));
static GUI_DEF(_bg1, GUI_GROUP(WG_ITEM(_rb0, 250, 14, -1, -1),
                               WG_ITEM(_rb1, 251, 28, -1, -1),
                               WG_ITEM(_rb2, 252, 42, -1, -1),
                               WG_ITEM(_rb3, 253, 56, -1, -1)));

static GUI_DEF(_t0, GUI_BUTTON(GUI_IMAGE("toggle_0.ilbm")));
static GUI_DEF(_l0, GUI_LABEL_N(40));
static GUI_DEF(_f0, GUI_FRAME(FRAME_FLAT, _l0));
static GUI_DEF(_root, GUI_GROUP(WG_ITEM(_bg0, 0, 0, -1, -1),
                                WG_ITEM(_bg1, 0, 0, -1, -1),
                                WG_ITEM(_f0, 0, 192, WIDTH, -1),
                                WG_ITEM(_t0, 160, 20, -1, -1)));
static GUI_MAIN(_root);

static void Load() {
  font = LoadFont("koi8r.8x8.font");
  GuiInit(gui, font);
}

static void UnLoad() {
  DeleteFont(font);
}

static void Init() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);
  pointer = CloneSystemPointer();

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetupSprites(cp, sprptr);
  CopSetRGB(cp, UI_BG_INACTIVE, 0x888);
  CopSetRGB(cp, UI_BG_ACTIVE,   0xaaa);
  CopSetRGB(cp, UI_FRAME_IN,    0x444);
  CopSetRGB(cp, UI_FRAME_OUT,   0xeee);
  CopSetRGB(cp, UI_FG_INACTIVE, 0x24a);
  CopSetRGB(cp, UI_FG_ACTIVE,   0x46e);
  CopEnd(cp);

  CopInsSet32(sprptr[0], pointer->data);
  UpdateSprite(pointer, X(0), Y(0));

  CopListActivate(cp);

  KeyboardInit();
  MouseInit(0, 0, WIDTH - 1, HEIGHT - 1);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE;

  GuiRedraw(gui, screen);
}

static void Kill() {
  custom->dmacon = DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE;

  KeyboardKill();
  MouseKill();

  DeleteSprite(pointer);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static BOOL HandleEvent() {
  EventT ev[1];

  if (!PopEvent(ev))
    return TRUE;

  if (ev->type == EV_KEY) {
   if (!(ev->key.modifier & MOD_PRESSED) && ev->key.code == KEY_ESCAPE)
      return FALSE;
  } else if (ev->type == EV_MOUSE) {
    GuiHandleMouseEvent(gui, &ev->mouse);
    UpdateSprite(pointer, X(ev->mouse.x), Y(ev->mouse.y));
  } else if (ev->type == EV_GUI) {
    WidgetT *wg = ev->gui.widget;

    if (wg->type == WT_BUTTON || wg->type == WT_RADIOBT) {
      wg = wg->button.widget;

      if (wg->type == WT_LABEL) {
        LabelT *lb = &wg->label;
        static char *action[] = {"Entered", "Left", "Pressed", "Released"};

        LabelFmtStr(_l0, "%s button '%s'!", action[ev->gui.action], lb->text);
        GuiWidgetRedraw(gui, _l0);
      }
    }
  }

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL, HandleEvent };
