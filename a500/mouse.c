#include "hardware.h"
#include "interrupts.h"
#include "coplist.h"
#include "sprite.h"
#include "mouse.h"
#include "keyboard.h"
#include "print.h"

static BitmapT *screen;
static CopListT *cp;
static SpriteT *pointer;
static SpriteT *nullspr;
static CopInsT *sprptr[8];

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);
  nullspr = NewSprite(0, FALSE);
  pointer = CloneSystemPointer();

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  CopMakeSprites(cp, sprptr, nullspr);
  CopEnd(cp);

  CopInsSet32(sprptr[0], pointer->data);
  UpdateSpritePos(pointer, 0x81, 0x2c);
}

void Kill() {
  DeleteSprite(pointer);
  DeleteSprite(nullspr);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

__interrupt_handler void IntLevel2Handler() {
  if (custom->intreqr & INTF_PORTS) {
    /* Make sure all scratchpad registers are saved, because we call a function
     * that relies on the fact that it's caller responsibility to save them. */
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    KeyboardIntHandler();
  }

  custom->intreq = INTF_PORTS;
  custom->intreq = INTF_PORTS;
}

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    /* Make sure all scratchpad registers are saved, because we call a function
     * that relies on the fact that it's caller responsibility to save them. */
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    MouseIntHandler();
  }

  custom->intreq = INTF_VERTB;
  custom->intreq = INTF_VERTB;
}

void Main() {
  APTR OldIntLevel2, OldIntLevel3;

  KeyboardInit();
  MouseInit(0, 0, screen->width - 1, screen->height - 1);

  OldIntLevel2 = InterruptVector->IntLevel2;
  InterruptVector->IntLevel2 = IntLevel2Handler;
  OldIntLevel3 = InterruptVector->IntLevel3;
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_PORTS | INTF_VERTB;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;

  while (1) {
    MouseEventT cursor;
    KeyEventT key;

    if (GetKeyEvent(&key)) {
      if (key.modifier & MOD_PRESSED)
        continue;
      if (key.code == KEY_ESCAPE)
        break;
    }

    if (GetMouseEvent(&cursor)) {
      UBYTE *data = screen->planes[0] + 
        (cursor.x + cursor.y * screen->width) / 8;
      UBYTE value = 1 << (7 - (cursor.x & 7));

      if (cursor.button & LMB_PRESSED)
        *data |= value;
      if (cursor.button & RMB_PRESSED)
        *data &= ~value;

      UpdateSpritePos(pointer, 0x81 + cursor.x, 0x2c + cursor.y);
    }
  }

  custom->intena = INTF_PORTS | INTF_VERTB;
  InterruptVector->IntLevel2 = OldIntLevel2;
  InterruptVector->IntLevel3 = OldIntLevel3;
}
