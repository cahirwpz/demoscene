#include <effect.h>
#include <copper.h>
#include <system/event.h>
#include <system/keyboard.h>

#define S_WIDTH 320
#define S_HEIGHT 256
#define S_DEPTH 7

#define WIDTH 64
#define HEIGHT 64

#include "data/dragon-bg.c"
#include "data/speccy.c"

static CopListT *cp;

int active_glitch = 0;

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

static void MakeCopperList(CopListT *cp) {
  short i;
  int mask;
  CopListReset(cp);
  CopSetupBitplanes(cp, &background, S_DEPTH);

  switch (active_glitch) {
    case 0: // TV_STATIC
      CopMove16(cp, bpldat[4], 0xffff);
      for (i = 0; i < S_HEIGHT; i++) {
        CopWaitSafe(cp, Y(i), 0);
        CopMove16(cp, bpldat[5], random());
      }
    case 1: // HALFBRITE
      mask = random();
      CopMove16(cp, bpldat[5], 0);
      CopWaitSafe(cp, Y(0), 0);
      CopMove16(cp, bpldat[4], mask);
  }

  CopListFinish(cp);
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);
  LoadColors(background_colors, 16);
  LoadColors(speccy_colors, 0);

  cp = NewCopList(800);
  MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);
  KeyboardInit();
}

static void Kill(void) {
  KeyboardKill();
  DisableDMA(DMAF_COPPER | DMAF_RASTER);
  DeleteCopList(cp);
}

PROFILE(UVMapRender);

static bool HandleEvent(void) {
  EventT ev;

  if (!PopEvent(&ev))
    return true;

  if (ev.key.code == KEY_1) {
    active_glitch = 0; // TV_STATIC
  }

  if (ev.key.code == KEY_2) {
    active_glitch = 1; // HALFBRITE
  }

  return true;
}

static void Render(void) {
  ProfilerStart(UVMapRender);
  {
    CopListActivate(cp);
  }
  ProfilerStop(UVMapRender);

  
  TaskWaitVBlank();
  MakeCopperList(cp);
  HandleEvent();
}

EFFECT(HBPlane, NULL, NULL, Init, Kill, Render, NULL);
