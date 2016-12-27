#include <proto/graphics.h>

#include "startup.h"
#include "io.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "ahx.h"
#include "console.h"
#include "coplist.h"
#include "keyboard.h"
#include "event.h"
#include "blitter.h"

LONG __chipmem = 100 * 1024;
LONG __fastmem = 420 * 1024;
STRPTR __cwdpath = "data";

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static APTR module;
static BitmapT *screen;
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

typedef struct {
  BitmapT *bm;
  BYTE *samples;
  WORD volume;
  LONG i, di;
} WaveScopeChanT;

#define TICKS_PER_FRAME (3579546L / 50)
#define WS_SAMPLES 64

static struct {
  WaveScopeChanT channel[4];
  BitmapT *spans;
  UBYTE multab[256*64];
} wavescope;

static void InitWaveScope() {
  WORD i, j;
  BitmapT *bm;

  for (i = 0; i < 4; i++) {
    memset(&wavescope.channel[i], 0, sizeof(WaveScopeChanT));
    wavescope.channel[i].bm = NewBitmap(64, 64, 1);
  }

  for (i = 0; i < 64; i++) {
    for (j = 0; j < 128; j++) {
      WORD index = (i << 8) | j;
      WORD x = (j * (i + 1)) >> 4;
      wavescope.multab[index] = x;
    }

    for (; j < 255; j++) {
      WORD index = (i << 8) | j;
      WORD x = ((255 - j) * (i + 1)) >> 4;
      wavescope.multab[index] = x;
    }
  }

  bm = NewBitmap(64, 32, 1);
  BlitterLineSetup(bm, 0, LINE_EOR, LINE_ONEDOT);
  BlitterLine(32, 0, 1, 31);
  BlitterLine(32, 0, 63, 31);
  BlitterFill(bm, 0);
  WaitBlitter();
  ((BYTE *)bm->planes[0])[4] = 0x80;
  wavescope.spans = bm;
}

static void KillWaveScope() {
  WORD i;

  for (i = 0; i < 4; i++)
    DeleteBitmap(wavescope.channel[i].bm);
  DeleteBitmap(wavescope.spans);
}

static void WaveScopeUpdateChannel(WORD num, AhxVoiceTempT *voice) {
  WaveScopeChanT *ch = &wavescope.channel[num];
  LONG samplesPerFrame = 1;

  if (voice->AudioPeriod)
    samplesPerFrame = TICKS_PER_FRAME / voice->AudioPeriod;

  ch->samples = voice->AudioPointer;
  ch->volume = voice->AudioVolume;
  ch->di = (samplesPerFrame << 16) / WS_SAMPLES;

  ch->volume -= 1;
  if (ch->volume < 0)
    ch->volume = 0;

  if (ch->samples != voice->AudioPointer)
    ch->i = 0;
}

static void WaveScopeDrawChannel(WORD num) {
  WaveScopeChanT *ch = &wavescope.channel[num];
  BitmapT *bm = ch->bm;
  LONG *dst = bm->planes[0];
  UBYTE *samples;
  UBYTE *multab;
  LONG i, di, volume;
  WORD n;

  BitmapClear(bm);

  multab = wavescope.multab;
  samples = ch->samples;
  volume = ch->volume << 8;
  i = ch->i, di = ch->di;

  for (n = 0; n < WS_SAMPLES; n++) {
    WORD x;

    i = swap16(i);
    x = multab[samples[i] | volume];
    i = swap16(i);

    i += di;
    if (i >= (AHX_SAMPLE_LEN << 16))
      i -= (AHX_SAMPLE_LEN << 16);

    {
      LONG *src = (LONG *)(wavescope.spans->planes[0] + (x & ~7));

      *dst++ = *src++;
      *dst++ = *src++;
    }
  }

  ch->i = i;
}

static void Load() {
  module = LoadFile("jazzcat-electric_city.ahx", MEMF_PUBLIC);
  if (AhxInitPlayer(AHX_LOAD_WAVES_FILE, AHX_FILTERS) != 0)
    exit(10);
}

static void UnLoad() {
  AhxKillPlayer();
  MemFree(module);
}

static __interrupt LONG AhxPlayerIntHandler() {
  /* Handle CIA Timer A interrupt. */
  if (ReadICR(ciaa) & CIAICRF_TA) {
    custom->color[0] = 0x448;
    AhxInterrupt();
    custom->color[0] = 0;
  }

  return 0;
}

INTERRUPT(AhxPlayerInterrupt, 10, AhxPlayerIntHandler);

static void AhxSetTempo(UWORD tempo asm("d0")) {
  ciaa->ciatalo = tempo & 0xff;
  ciaa->ciatahi = tempo >> 8;
  ciaa->ciacra |= CIACRAF_START;
}

static void DrawFrames() {
  WORD i;

  BlitterLineSetup(screen, 0, LINE_OR, LINE_SOLID);

  for (i = 0; i < 4; i++) {
    WORD x1 = 8 + 72 * i - 1;
    WORD x2 = 72 + 72 * i + 1;
    WORD y1 = 96 - 1;
    WORD y2 = 160 + 1;

    BlitterLine(x1, y1, x2, y1);
    BlitterLine(x1, y2, x2, y2);
    BlitterLine(x1, y1, x1, y2);
    BlitterLine(x2, y1, x2, y2);
  }
}

static void Init() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);
  CopListActivate(cp);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }

  ConsoleInit(&console, screen, topaz8);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  ConsolePutStr(&console, 
                "Pause (SPACE) Prev (LEFT) Next (RIGHT)\n"
                "Exit (ESC)\n");
  DrawFrames();
  InitWaveScope();
  KeyboardInit();

  if (AhxInitHardware((APTR)AhxSetTempo, AHX_KILL_SYSTEM) == 0)
    if (AhxInitModule(module) == 0)
      AhxInitSubSong(0, 0);

  AddIntServer(INTB_PORTS, &AhxPlayerInterrupt);
}

static void Kill() {
  RemIntServer(INTB_PORTS, &AhxPlayerInterrupt);

  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;

  KillWaveScope();
  AhxStopSong();
  AhxKillHardware();

  CloseFont(topaz8);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void Render() {
  // LONG lines = ReadLineCounter();
  WORD i;

  ConsoleSetCursor(&console, 0, 3);
  ConsolePrint(&console, "Position : %02ld/%02ld\n\n",
               (LONG)Ahx.Public->Pos, (LONG)Ahx.Public->Row);

  if (Ahx.Public->Playing) {
    WaitLine(Y(96));

    for (i = 0; i < 4; i++) {
      WORD x = 8 + 72 * i;
      WORD y = 96;

      WaveScopeUpdateChannel(i, &Ahx.Public->VoiceTemp[i]);
      WaveScopeDrawChannel(i);
      BitmapCopy(screen, x, y, wavescope.channel[i].bm);
    }
  }
  
  // Log("playahx: %ld\n", ReadLineCounter() - lines);
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
    Ahx.Public->Playing = ~Ahx.Public->Playing;
    custom->dmacon = (Ahx.Public->Playing ? DMAF_SETCLR : 0) | DMAF_AUDIO;
  }

  if (ev.key.code == KEY_LEFT)
    AhxPrevPattern();

  if (ev.key.code == KEY_RIGHT)
    AhxNextPattern();

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, HandleEvent };
