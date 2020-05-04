#include "startup.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "cinter.h"
#include "console.h"
#include "copper.h"
#include "keyboard.h"
#include "event.h"
#include "blitter.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

#include "data/lat2-08.c"

extern u_char binary_data_JazzCat_Automatic_ctr_start[];
#define module binary_data_JazzCat_Automatic_ctr_start

extern u_char binary_data_JazzCat_Automatic_smp_start[];
#define samples binary_data_JazzCat_Automatic_smp_start

extern u_char binary_data_JazzCat_Automatic_smp_size[];
#define samples_len ((long)binary_data_JazzCat_Automatic_smp_size)

static void *instruments;
static CinterPlayerT *player;
static BitmapT *screen;
static CopListT *cp;
static ConsoleT console;

/* Extra variables to enhance replayer functionality */
static bool stopped = true;
static u_short *musicStart;

/* Total instrument memory is determined by converter script! */
#define INSTRUMENTS_TOTAL 153172

static void Load(void) {
  Log("Raw samples length: %d\n", (int)samples_len);
  instruments = MemAlloc(INSTRUMENTS_TOTAL, MEMF_CHIP|MEMF_CLEAR);
  player = MemAlloc(sizeof(CinterPlayerT), MEMF_FAST|MEMF_CLEAR);
  memcpy(instruments, samples, samples_len);
}

static void UnLoad(void) {
  MemFree(player);
  MemFree(instruments);
  MemFree(module);
}

static __interrupt int CinterMusic(void) {
  if (stopped)
    return 0;
  CinterPlay1(player);
  CinterPlay2(player);
  return 0;
}

INTERRUPT(CinterPlayerInterrupt, 10, CinterMusic, NULL);

static void Init(void) {
  KeyboardInit();

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0xfff);
  CopEnd(cp);

  ConsoleInit(&console, &latin2, screen);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  ConsoleSetCursor(&console, 0, 0);
  ConsolePutStr(&console, "Initializing Cinter... please wait!\n");
  CinterInit(module, instruments, player);
  musicStart = player->c_MusicPointer;

  AddIntServer(INTB_VERTB, CinterPlayerInterrupt);

  ConsoleSetCursor(&console, 0, 0);
  ConsolePutStr(&console, 
                "Pause (SPACE) -10s (LEFT) +10s (RIGHT)\n"
                "Exit (ESC)\n");

  stopped = false;
}

static void Kill(void) {
  RemIntServer(INTB_VERTB, CinterPlayerInterrupt);

  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_AUDIO);

  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static bool HandleEvent(void);

static void Render(void) {
  short i;
  short trackSize = player->c_TrackSize / 2;
  CinterChannelT *channel = player->c_MusicState;
  u_short *music = player->c_MusicPointer;

  ConsoleSetCursor(&console, 0, 3);
  ConsolePrint(&console, "Cinter Player\n\n");

  for (i = 0; i < 4; i++, channel++) {
    ConsolePrint(&console, "CH%d: %8x %p %4x %4x\n",
                 i, channel->state, channel->sample,
                 channel->period, channel->volume);
  }

  ConsolePrint(&console, "\n");

  for (i = 0; i < 16; i++, music++) {
    u_short m0 = music[0 * trackSize];
    u_short m1 = music[1 * trackSize];
    u_short m2 = music[2 * trackSize];
    u_short m3 = music[3 * trackSize];
    int pos = (void *)music - (void *)musicStart;

    ConsolePrint(&console, "%4x %4x %4x %4x %4x\n", pos, m0, m1, m2, m3);
  }

  TaskWaitVBlank();

  exitLoop = !HandleEvent();
}

static bool HandleEvent(void) {
  EventT ev;

  if (!PopEvent(&ev))
    return true;

  if (ev.type == EV_MOUSE)
    return true;

  if (ev.key.modifier & MOD_PRESSED)
    return true;

  if (ev.key.code == KEY_ESCAPE)
    return false;

  if (ev.key.code == KEY_SPACE) {
    stopped ^= 1;
    if (stopped)
      DisableDMA(DMAF_AUDIO);
  }

  if (ev.key.code == KEY_LEFT) {
    u_short *current = player->c_MusicPointer - 500;
    if (current < musicStart)
      current = musicStart;
    player->c_MusicPointer = current;
  }

  if (ev.key.code == KEY_RIGHT) {
    u_short *current = player->c_MusicPointer + 500;
    if (current < player->c_MusicEnd)
      player->c_MusicPointer = current;
  }

  return true;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
