#include "startup.h"
#include "hardware.h"
#include "interrupts.h"
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
static SpriteT *nullspr;
static CopInsT *sprptr[8];

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  cp = NewCopList(100);
  nullspr = NewSprite(0, FALSE);
  pointer = CloneSystemPointer();

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakeSprites(cp, sprptr, nullspr);
  CopEnd(cp);

  CopInsSet32(sprptr[0], pointer->data);
  UpdateSpritePos(pointer, X(0), Y(0));
}

static void Kill() {
  DeleteSprite(pointer);
  DeleteSprite(nullspr);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static __interrupt_handler void IntLevel2Handler() {
  if (custom->intreqr & INTF_PORTS) {
    /* Make sure all scratchpad registers are saved, because we call a function
     * that relies on the fact that it's caller responsibility to save them. */
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    KeyboardIntHandler();
  }

  custom->intreq = INTF_PORTS;
  custom->intreq = INTF_PORTS;
}

static __interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    /* Make sure all scratchpad registers are saved, because we call a function
     * that relies on the fact that it's caller responsibility to save them. */
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    MouseIntHandler();
  }

  custom->intreq = INTF_VERTB;
  custom->intreq = INTF_VERTB;
}

static void Init() {
  KeyboardInit();
  MouseInit(0, 0, WIDTH - 1, HEIGHT - 1);

  InterruptVector->IntLevel2 = IntLevel2Handler;
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_PORTS | INTF_VERTB;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;
}

static void Loop() {
  BOOL quit = FALSE;

  while (!quit) {
    MouseEventT cursor;
    KeyEventT key;

    if (GetKeyEvent(&key)) {
      if (!(key.modifier & MOD_PRESSED) && (key.code == KEY_ESCAPE))
        quit = TRUE;
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
  }
}

EffectT Effect = { Load, Init, Kill, Loop };
