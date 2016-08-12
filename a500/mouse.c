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
static GUI_BUTTON(_b0,   0,  0, 48, 12, "Play");
static GUI_BUTTON(_b1,  48, 20, 64, 12, "Pause");
static GUI_BUTTON(_b2,  96, 40, 48, 12, "Stop");
static GUI_BUTTON(_b3, 144, 60, 80, 12, "Forward");
static GUI_BUTTON(_b4, 192, 80, 80, 12, "Reverse");
static GUI_GROUP(_bg0, _b0, _b1, _b2, _b3, _b4);

static GUI_RADIOBT(_rb0, 250, 14, 12, 12, "A");
static GUI_RADIOBT(_rb1, 251, 28, 12, 12, "B");
static GUI_RADIOBT(_rb2, 252, 42, 12, 12, "C");
static GUI_RADIOBT(_rb3, 253, 56, 12, 12, "D");
static GUI_GROUP(_bg1, _rb0, _rb1, _rb2, _rb3);

static GUI_LABEL(_l0, 0, 192, 320, 12, FRAME_FLAT, 40);
static GUI_GROUP(_root, _bg0, _bg1, _l0);
static GUI_MAIN(_root);

static void Load() {
  font = LoadFont("koi8r.8x8.font");
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

  GuiInit(gui, screen, font);
  GuiRedraw(gui);
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
      ButtonT *bt = &wg->button;
      static char *action[] = {"Entered", "Left", "Pressed", "Released"};

      LabelFmtStr(_l0, "%s button '%s'!", action[ev->gui.action], bt->label);
      GuiWidgetRedraw(gui, _l0);
    }
  }

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL, HandleEvent };
