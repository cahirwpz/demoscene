#include "effect.h"
#include "console.h"
#include "copper.h"
#include <system/event.h>
#include <system/keyboard.h>
#include <system/file.h>
#include <system/serial.h>

#define WIDTH 640
#define HEIGHT 256
#define DEPTH 1

#include "data/drdos8x8.c"

static BitmapT *screen;
static CopListT *cp;
static ConsoleT console;
static FileT *ser;

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  cp = NewCopList(100);

  SetupPlayfield(MODE_HIRES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, 0x000);
  SetColor(1, 0xfff);

  CopInit(cp);
  CopSetupBitplanes(cp, screen, DEPTH);
  CopEnd(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  ConsoleInit(&console, &drdos8x8, screen);
  ConsolePutStr(&console, "Press ESC key to exit!\n");
  ConsoleDrawCursor(&console);

  ser = OpenSerial(9600, O_NONBLOCK);
  KeyboardInit();
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);

  KeyboardKill();
  FileClose(ser);

  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static bool HandleEvent(void) {
  EventT ev;
  int c = FileGetChar(ser);

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
    FilePutChar(ser, ev.key.ascii);
  }

  ConsoleDrawCursor(&console);

  return true;
}

static void Render(void) {
  exitLoop = !HandleEvent();
}

EFFECT(KbdTest, NULL, NULL, Init, Kill, Render, NULL);
