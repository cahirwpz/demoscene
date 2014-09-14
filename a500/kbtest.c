#include <proto/graphics.h>

#include "startup.h"
#include "console.h"
#include "hardware.h"
#include "interrupts.h"
#include "coplist.h"
#include "keyboard.h"

#define WIDTH 640
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen, DEPTH);
  CopMakeDispWinHiRes(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }

  ConsoleInit(&console, screen, topaz8);
}

static void UnLoad() {
  CloseFont(topaz8);
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

static void Init() {
  KeyboardInit();
  InterruptVector->IntLevel2 = IntLevel2Handler;
  custom->intena = INTF_SETCLR | INTF_PORTS;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  ConsolePutStr(&console, "Press ESC key to exit!\n");
  ConsoleDrawCursor(&console);
}

static BOOL HandleEvent() {
  KeyEventT event;

  if (!GetKeyEvent(&event))
    return TRUE;

  if (event.modifier & MOD_PRESSED)
    return TRUE;

  if (event.code == KEY_ESCAPE)
    return FALSE;

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

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, NULL, NULL, HandleEvent };
