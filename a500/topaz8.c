#include <proto/graphics.h>

#include "blitter.h"
#include "coplist.h"
#include "console.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

static BitmapT *screen;
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);

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
  CopInit(cp);

  CopMove16(cp, bplcon0, BPLCON0_BPU(screen->depth) | BPLCON0_COLOR);
  CopMove16(cp, bplcon1, 0);
  CopMove16(cp, bplcon2, 0);
  CopMove32(cp, bplpt[0], screen->planes[0]);

  CopMove16(cp, ddfstrt, 0x38);
  CopMove16(cp, ddfstop, 0xd0);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopMove16(cp, color[0], 0x000);
  CopMove16(cp, color[1], 0xfff);

  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_MASTER;

  ConsoleDrawBox(&console, 10, 10, 20, 20);
  ConsoleSetCursor(&console, 2, 2);
  ConsolePutStr(&console, "The quick brown fox jumps\nover the lazy dog\n");

  WaitMouse();
}
