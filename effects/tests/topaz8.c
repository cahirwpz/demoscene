#include <exec/execbase.h>
#include <proto/exec.h>

#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "console.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;
static ConsoleT console;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  ConsoleInit(&console, screen);
}

static void UnLoad() {
  ConsoleKill(topaz8);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void Init() {
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  ConsoleDrawBox(&console, 10, 10, 20, 20);
  ConsoleSetCursor(&console, 2, 2);
  ConsolePrint(&console, "Running on Kickstart %d.%d.\n",
               SysBase->LibNode.lib_Version,
               SysBase->LibNode.lib_Revision);
  ConsolePutStr(&console, "The quick brown fox jumps\nover the lazy dog\n");
}

EffectT Effect = { Load, UnLoad, Init, NULL, NULL };
