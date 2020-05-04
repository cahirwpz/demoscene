#include "startup.h"
#include "console.h"
#include "hardware.h"
#include "coplist.h"
#include "event.h"
#include "keyboard.h"
#include "serial.h"

#define WIDTH 640
#define HEIGHT 256
#define DEPTH 1

#include "data/drdos8x8.c"

static BitmapT *screen;
static CopListT *cp;
static ConsoleT console;

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_HIRES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0xfff);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  ConsoleInit(&console, &drdos8x8, screen);
  ConsolePutStr(&console, "Press ESC key to exit!\n");
  ConsoleDrawCursor(&console);

  SerialInit(9600);
  KeyboardInit();
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);

  KeyboardKill();
  SerialKill();

  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static bool HandleEvent(void) {
  EventT ev;
  int c = SerialGet();

  if (c >= 0) {
    ConsolePutChar(&console, c);
    ConsoleDrawCursor(&console);
  }

  if (!PopEvent(&ev))
    return true;

  if (ev.type != EV_KEY)
    return true;

  if (ev.key.modifier & MOD_PRESSED)
    return true;

  if (ev.key.code == KEY_ESCAPE)
    return false;

  ConsoleDrawCursor(&console);

  if (ev.key.code == KEY_LEFT) {
    if (console.cursor.x > 0)
      console.cursor.x--;
  }

  if (ev.key.code == KEY_RIGHT) {
    console.cursor.x++;
    if (console.cursor.x >= console.width)
      console.cursor.x = console.width;
  }

  if (ev.key.code == KEY_UP) {
    if (console.cursor.y > 0)
      console.cursor.y--;
  }

  if (ev.key.code == KEY_DOWN) {
    console.cursor.y++;
    if (console.cursor.y >= console.height)
      console.cursor.y = console.height;
  }

  if (ev.key.ascii) {
    ConsolePutChar(&console, ev.key.ascii);
    SerialPut(ev.key.ascii);
  }

  ConsoleDrawCursor(&console);

  return true;
}

static void Render(void) {
  exitLoop = !HandleEvent();
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
