#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "sprite.h"
#include "mouse.h"
#include "keyboard.h"
#include "print.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;
static SpriteT *pointer;
static CopInsT *sprptr[8];

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);
  pointer = CloneSystemPointer();

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopMakeSprites(cp, sprptr);
  CopEnd(cp);

  CopInsSet32(sprptr[0], pointer->data);
  UpdateSpritePos(pointer, X(0), Y(0));
}

static void UnLoad() {
  DeleteSprite(pointer);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void Init() {
  KeyboardInit();
  MouseInit(0, 0, WIDTH - 1, HEIGHT - 1);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;
}

static void Kill() {
  custom->dmacon = DMAF_RASTER | DMAF_SPRITE;
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

    UpdateSpritePos(pointer, X(cursor.x), Y(cursor.y));
  }

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL, NULL, HandleEvent };
