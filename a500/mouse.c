#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "sprite.h"
#include "event.h"
#include "io.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;
static SpriteT *pointer;
static CopInsT *sprptr[8];

static void Init() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);
  pointer = CloneSystemPointer();

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetupSprites(cp, sprptr);
  CopEnd(cp);

  CopInsSet32(sprptr[0], pointer->data);
  UpdateSprite(pointer, X(0), Y(0));

  CopListActivate(cp);

  KeyboardInit();
  MouseInit(0, 0, WIDTH - 1, HEIGHT - 1);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;
}

static void Kill() {
  custom->dmacon = DMAF_RASTER | DMAF_SPRITE;

  KeyboardKill();
  MouseKill();

  DeleteSprite(pointer);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static BOOL HandleEvent() {
  EventT ev;

  if (!PopEvent(&ev))
    return TRUE;

  if (ev.type == EV_KEY)
   if (!(ev.key.modifier & MOD_PRESSED) && ev.key.code == KEY_ESCAPE)
      return FALSE;

  if (ev.type == EV_MOUSE) {
    UBYTE *data = screen->planes[0] + 
      ev.mouse.x / 8 + ev.mouse.y * screen->bytesPerRow;
    UBYTE value = 1 << (7 - (ev.mouse.x & 7));

    if (ev.mouse.button & LMB_PRESSED)
      *data |= value;
    if (ev.mouse.button & RMB_PRESSED)
      *data &= ~value;

    UpdateSprite(pointer, X(ev.mouse.x), Y(ev.mouse.y));
  }

  return TRUE;
}

EffectT Effect = { NULL, NULL, Init, Kill, NULL, HandleEvent };
