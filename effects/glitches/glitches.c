#include <effect.h>
#include <copper.h>
#include <system/event.h>
#include <system/keyboard.h>

#define S_WIDTH 320
#define S_HEIGHT 256
#define S_DEPTH 7

#define WIDTH 64
#define HEIGHT 64
#define DEPTH 4

#include "data/dragon-bg.c"
#include "data/speccy.c"
#include "data/tearing-table.c"

static int active = 1;

// Since for each glitch the copper list looks
// a bit different, a pair of coplists is generated
// for every glitch.
// 0 - tv static
// 1 - halfbrite
// 2 - tearing
static CopListT *cp[3][2];

typedef struct State {
  CopInsT *tvstatic;
  CopInsT *halfbrite;
  CopInsT *tearing;
} StateT;
static StateT state[2];

int active_glitch = 0;
int animationFrame = -1;
short offset = 70;

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

#define COPLIST_SIZE (HEIGHT * 22 + 100)

static CopListT *MakeCopperListStatic(StateT *state) {
  short i;
  CopListT *cp = NewCopList(COPLIST_SIZE);
  CopSetupBitplanes(cp, &background, S_DEPTH);

  state->tvstatic = CopMove16(cp, bplcon1, 0x0000);
  CopMove16(cp, bpldat[4], 0xffff);
  CopMove16(cp, bpldat[5], 0x00);
  for (i = 0; i < S_HEIGHT; i++) {
    CopWaitSafe(cp, Y(i), 0);
    CopMove16(cp, bpldat[5], NULL);
  }

  return CopListFinish(cp);
}

static CopListT *MakeCopperListHalfbrite(StateT *state) {
  CopListT *cp = NewCopList(COPLIST_SIZE);
  CopSetupBitplanes(cp, &background, S_DEPTH);

  state->halfbrite = CopMove16(cp, bplcon1, 0x0000);
  CopMove16(cp, bpldat[5], 0x00);
  CopWaitSafe(cp, Y(0), 0);
  CopMove16(cp, bpldat[4], 0xffff);

  return CopListFinish(cp);
}

// Still a bit glitchy, bad instruction offsets?
static CopListT *MakeCopperListTearing(StateT *state) {
  short i;
  CopListT *cp = NewCopList(COPLIST_SIZE);
  CopSetupBitplanes(cp, &background, S_DEPTH);

  state->tearing = CopMove16(cp, bplcon1, 0x0000);
  CopMove16(cp, bpldat[4], 0x00);
  CopMove16(cp, bpldat[5], 0x00);
  for (i = 0; i < 15; i++) {
    CopWaitSafe(cp, Y(i+70), 0);
    CopMove16(cp, bplcon1, NULL);
  }
  for (i = 16; i < 31; i++) { // TODO: clean up into one for loop
    CopWaitSafe(cp, Y(i+offset), 0);
    CopMove16(cp, bplcon1, NULL);
  }
  CopWaitSafe(cp, Y(31+offset), 0);
  CopMove16(cp, bplcon1, 0);

  return CopListFinish(cp);
}

static void UpdateCopperList(StateT *state) {
  switch (active_glitch) {
    CopInsT *ins;
    short i;
    case 0: // TV_STATIC
      ins = state->tvstatic;
      CopInsSet16(&ins[0], 0x00);
      CopInsSet16(&ins[1], 0x00);
      for (i = 4; i < S_HEIGHT * 2; i += 2) {
        CopInsSet16(&ins[i], random());
      }
      break;
    case 1: // HALFBRITE
      ins = state->halfbrite;
      CopInsSet16(&ins[0], 0x00);
      CopInsSet16(&ins[1], 0x00);
      CopInsSet16(&ins[3], random());
      break;
    case 2: // TEARING
      ins = state->tearing;
      CopInsSet16(&ins[0], 0x00);
      CopInsSet16(&ins[1], 0x00);
      CopInsSet16(&ins[2], 0x00);
      if (animationFrame != -1) {
        for (i = 0; i < 30; i += 2) {
          CopInsSet16(&ins[2 + i], tears[i][animationFrame]);
        }
        for (i = 31; i < 62; i += 2) {
          CopInsSet16(&ins[2 + i], tears[31-i][animationFrame]);
        }
        CopInsSet16(&ins[63], 0);
        
      }
      break;
  }
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);
  LoadColors(background_colors, 0);
  LoadColors(speccy_colors, 16);

  cp[0][0] = MakeCopperListStatic(&state[0]);
  cp[0][1] = MakeCopperListStatic(&state[1]);
  cp[1][0] = MakeCopperListHalfbrite(&state[0]);
  cp[1][1] = MakeCopperListHalfbrite(&state[1]);
  cp[2][0] = MakeCopperListTearing(&state[0]);
  cp[2][1] = MakeCopperListTearing(&state[1]);
  CopListActivate(cp[0][0]);

  EnableDMA(DMAF_RASTER);
  KeyboardInit();
}

static void Kill(void) {
  KeyboardKill();
  DisableDMA(DMAF_COPPER | DMAF_RASTER);
  DeleteCopList(cp[0][0]);
  DeleteCopList(cp[0][1]);
}

PROFILE(Glitches);

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

  if (ev.key.code == KEY_3) {
    active_glitch = 2; // TEARING
  }

  // if (ev.key.code == KEY_4) {
  //   active_glitch = 3; // SHAKING
  // }

  if (ev.key.code == KEY_A) {
    if (animationFrame == -1) {
      offset = random() & 255;
    }
    animationFrame = 16; // trigger animation
  }

  return true;
}

static void Render(void) {
  ProfilerStart(Glitches);
  {
    UpdateCopperList(&state[active]);
  }
  ProfilerStop(Glitches);

  if (animationFrame > -1) {
    animationFrame--;
  }
  
  HandleEvent();

  CopListRun(cp[active_glitch][active]);
  WaitVBlank();
  active ^= 1;
}

EFFECT(Glitches, NULL, NULL, Init, Kill, Render, NULL);
