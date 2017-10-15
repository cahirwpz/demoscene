#include <proto/graphics.h>

#include "startup.h"
#include "io.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "cinter.h"
#include "console.h"
#include "coplist.h"
#include "keyboard.h"
#include "event.h"
#include "blitter.h"
#include "tasks.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

STRPTR __cwdpath = "data";

static APTR module, instruments;
static CinterPlayerT *player;
static BitmapT *screen;
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

/* Extra variables to enhance replayer functionality */
static BOOL stopped = TRUE;
static UWORD *musicStart;

/* Total instrument memory is determined by converter script! */
#define INSTRUMENTS_TOTAL 153172

static void Load() {
  LONG samples_len = GetFileSize("JazzCat-Automatic.smp");
  APTR samples = LoadFile("JazzCat-Automatic.smp", MEMF_FAST);
  Log("Raw samples length: %ld\n", samples_len);
  module = LoadFile("JazzCat-Automatic.ctr", MEMF_FAST);
  instruments = MemAlloc(INSTRUMENTS_TOTAL, MEMF_CHIP|MEMF_CLEAR);
  player = MemAlloc(sizeof(CinterPlayerT), MEMF_FAST|MEMF_CLEAR);
  memcpy(instruments, samples, samples_len);
  MemFree(samples);
}

static void UnLoad() {
  MemFree(player);
  MemFree(instruments);
  MemFree(module);
}

static __interrupt LONG CinterMusic() {
  if (stopped)
    return 0;
  CinterPlay1(player);
  CinterPlay2(player);
  return 0;
}

INTERRUPT(CinterPlayerInterrupt, 10, CinterMusic, NULL);

static void Init() {
  KeyboardInit();

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }

  ConsoleInit(&console, screen, topaz8);

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

  stopped = FALSE;
}

static void Kill() {
  RemIntServer(INTB_VERTB, CinterPlayerInterrupt);

  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_AUDIO);

  CloseFont(topaz8);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void Render() {
  WORD i;
  WORD trackSize = player->c_TrackSize / 2;
  CinterChannelT *channel = player->c_MusicState;
  UWORD *music = player->c_MusicPointer;

  ConsoleSetCursor(&console, 0, 3);
  ConsolePrint(&console, "Cinter Player\n\n");

  for (i = 0; i < 4; i++, channel++) {
    ConsolePrint(&console, "CH%ld: %8lx %8lx %4lx %4lx\n",
                 (LONG)i, channel->state, (LONG)channel->sample,
                 (LONG)channel->period, (LONG)channel->volume);
  }

  ConsolePrint(&console, "\n");

  for (i = 0; i < 16; i++, music++) {
    UWORD m0 = music[0 * trackSize];
    UWORD m1 = music[1 * trackSize];
    UWORD m2 = music[2 * trackSize];
    UWORD m3 = music[3 * trackSize];
    LONG pos = (APTR)music - (APTR)musicStart;

    ConsolePrint(&console, "%4lx %4lx %4lx %4lx %4lx\n",
                 pos, (LONG)m0, (LONG)m1, (LONG)m2, (LONG)m3);
  }

  TaskWait(VBlankEvent);
}

static BOOL HandleEvent() {
  EventT ev;

  if (!PopEvent(&ev))
    return TRUE;

  if (ev.type == EV_MOUSE)
    return TRUE;

  if (ev.key.modifier & MOD_PRESSED)
    return TRUE;

  if (ev.key.code == KEY_ESCAPE)
    return FALSE;

  if (ev.key.code == KEY_SPACE) {
    stopped ^= 1;
    if (stopped)
      DisableDMA(DMAF_AUDIO);
  }

  if (ev.key.code == KEY_LEFT) {
    UWORD *current = player->c_MusicPointer - 500;
    if (current < musicStart)
      current = musicStart;
    player->c_MusicPointer = current;
  }

  if (ev.key.code == KEY_RIGHT) {
    UWORD *current = player->c_MusicPointer + 500;
    if (current < player->c_MusicEnd)
      player->c_MusicPointer = current;
  }

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, HandleEvent };
