#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <gfx.h>
#include <line.h>
#include <stdlib.h>
#include <system/memory.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen;

#include "data/anemone-pal.c"

static inline int fastrand(void) {
  static int m[2] = { 0x3E50B28C, 0xD461A7F9 };

  int a, b;

  // https://www.atari-forum.com/viewtopic.php?p=188000#p188000
  asm volatile("move.l (%2)+,%0\n"
               "move.l (%2),%1\n"
               "swap   %1\n"
               "add.l  %0,(%2)\n"
               "add.l  %1,-(%2)\n"
               : "=d" (a), "=d" (b)
               : "a" (m));
  
  return a;
}

#define random fastrand

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadPalette(&anemone_pal, 0);

  cp = NewCopList(50);
  CopInit(cp);
  CopSetupBitplanes(cp, bplptr, screen, DEPTH);
  CopEnd(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_BLITTER | DMAF_RASTER);

  DeleteBitmap(screen);
  DeleteCopList(cp);
}

#define screen_bytesPerRow (WIDTH / 8)

PROFILE(SeaAnemone);

static void Render(void) {
  ProfilerStart(SeaAnemone);
  ProfilerStop(SeaAnemone);

  TaskWaitVBlank();
}

EFFECT(SeaAnemone, NULL, NULL, Init, Kill, Render);
