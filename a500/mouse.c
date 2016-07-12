#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "sprite.h"
#include "gui.h"
#include "event.h"
#include "ilbm.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3

static BitmapT *screen;
static CopListT *cp;
static SpriteT *pointer;
static CopInsT *sprptr[8];
static BitmapT *font;

/* Test program */
static GUI_BUTTON(_b0, 0, 0, 48, 10, "Play");
static GUI_BUTTON(_b1, 48, 20, 64, 10, "Pause");
static GUI_BUTTON(_b2, 96, 40, 48, 10, "Stop");
static GUI_BUTTON(_b3, 144, 60, 80, 10, "Forward");
static GUI_BUTTON(_b4, 192, 80, 80, 10, "Reverse");
static GUI_LABEL(_l0, 0, 192, 320, 10, FRAME_FLAT, 40);
static GUI_GROUP(_buttons, _b0, _b1, _b2, _b3, _b4, _l0);
static GUI_MAIN(_buttons);

static void Load() {
  font = LoadILBMCustom("data/text-scroll-font.ilbm", 0);
}

static void UnLoad() {
  DeleteBitmap(font);
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

    if (wg->type == WT_BUTTON) {
      ButtonT *bt = &wg->button;
      static char *action[] = {"Entered", "Left", "Pressed", "Released"};

      LabelFmtStr(_l0, "%s button '%s'!", action[ev->gui.action], bt->label);
      GuiWidgetRedraw(gui, _l0);
    }
  }

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL, HandleEvent };
