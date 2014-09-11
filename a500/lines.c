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

#define CPULINE 1

void Main() {
  WORD i;

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  {
    LONG lines = ReadLineCounter();

#if CPULINE == 1
    CpuLineSetup(screen, 0);
#else
    BlitterLineSetup(screen, 0, LINE_OR, LINE_SOLID);
#endif

    for (i = 0; i < screen->width; i += 2) {
#if CPULINE == 1
      CpuLine(i, 0, screen->width - 1 - i, screen->height - 1);
#else
      BlitterLineSync(i, 0, screen->width - 1 - i, screen->height - 1);
#endif
    }

    for (i = 0; i < screen->height; i += 2) {
#if CPULINE == 1
      CpuLine(0, i, screen->width - 1, screen->height - 1 - i);
#else
      BlitterLineSync(0, i, screen->width - 1, screen->height - 1 - i);
#endif
    }

    Log("lines: %ld\n", ReadLineCounter() - lines);
  }

  WaitMouse();
}
