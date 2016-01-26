#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "sprite.h"
#include "mouse.h"
#include "keyboard.h"
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
  MouseEventT cursor;
  KeyEventT key;

  if (GetKeyEvent(&key)) {
    if (!(key.modifier & MOD_PRESSED) && (key.code == KEY_ESCAPE))
      return FALSE;
  }

  if (GetMouseEvent(&cursor)) {
    UBYTE *data = screen->planes[0] + 
      cursor.x / 8 + cursor.y * screen->bytesPerRow;
    UBYTE value = 1 << (7 - (cursor.x & 7));

    if (cursor.button & LMB_PRESSED)
      *data |= value;
    if (cursor.button & RMB_PRESSED)
      *data &= ~value;

    UpdateSprite(pointer, X(cursor.x), Y(cursor.y));
  }

  return TRUE;
}

EffectT Effect = { NULL, NULL, Init, Kill, NULL, HandleEvent };
