#include "blitter.h"
#include "coplist.h"
#include "line.h"

static BitmapT *screen;
static CopListT *cp;

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

#define CPULINE

void Main() {
  UWORD i;

  CopInit(cp);
  CopMakePlayfield(cp, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  for (i = 0; i < screen->width; i += 2) {
#ifdef CPULINE
    CpuLine(screen, 0, i, 0, screen->width - 1 - i, screen->height - 1);
#else
    BlitterLine(screen, 0, i, 0, screen->width - 1 - i, screen->height - 1);
    WaitBlitter();
#endif
  }

  for (i = 0; i < screen->height; i += 2) {
#ifdef CPULINE
    CpuLine(screen, 0, 0, i, screen->width - 1, screen->height - 1 - i);
#else
    BlitterLine(screen, 0, 0, i, screen->width - 1, screen->height - 1 - i);
    WaitBlitter();
#endif
  }

  WaitMouse();
}
