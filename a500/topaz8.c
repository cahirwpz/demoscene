#include <exec/execbase.h>
#include <proto/graphics.h>
#include <proto/exec.h>

#include "blitter.h"
#include "coplist.h"
#include "console.h"

static BitmapT *screen;
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
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

void Main() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  ConsoleDrawBox(&console, 10, 10, 20, 20);
  ConsoleSetCursor(&console, 2, 2);
  ConsolePrint(&console, "Running on Kickstart %ld.%ld.\n",
               (LONG)SysBase->LibNode.lib_Version,
               (LONG)SysBase->LibNode.lib_Revision);
  ConsolePutStr(&console, "The quick brown fox jumps\nover the lazy dog\n");

  WaitMouse();
}
