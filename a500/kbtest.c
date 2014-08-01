#include <proto/graphics.h>

#include "console.h"
#include "hardware.h"
#include "interrupts.h"
#include "coplist.h"
#include "keyboard.h"

static BitmapT *screen;
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

void Load() {
  screen = NewBitmap(640, 256, 1, FALSE);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width / 2, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }

  ConsoleInit(&console, screen, topaz8);
}

void Kill() {
  CloseFont(topaz8);
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

void Main() {
  KeyboardInit();
  InterruptVector->IntLevel2 = IntLevel2Handler;
  custom->intena = INTF_SETCLR | INTF_PORTS;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  ConsolePutStr(&console, "Press ESC key to exit!\n");
  ConsoleDrawCursor(&console);

  while (1) {
    KeyEventT event;

    if (GetKeyEvent(&event)) {
      if (event.modifier & MOD_PRESSED)
        continue;

      if (event.code == KEY_ESCAPE)
        break;

      ConsoleDrawCursor(&console);

      if (event.code == KEY_LEFT) {
        if (console.cursor.x > 0)
          console.cursor.x--;
      }

      if (event.code == KEY_RIGHT) {
        console.cursor.x++;
        if (console.cursor.x >= console.width)
          console.cursor.x = console.width;
      }

      if (event.code == KEY_UP) {
        if (console.cursor.y > 0)
          console.cursor.y--;
      }

      if (event.code == KEY_DOWN) {
        console.cursor.y++;
        if (console.cursor.y >= console.height)
          console.cursor.y = console.height;
      }

      if (event.ascii)
        ConsolePutChar(&console, event.ascii);

      ConsoleDrawCursor(&console);
    }
  }
}
