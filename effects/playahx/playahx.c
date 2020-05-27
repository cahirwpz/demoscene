#include "effect.h"
#include "hardware.h"
#include "interrupts.h"
#include "memory.h"
#include "ahx.h"
#include "console.h"
#include "copper.h"
#include "keyboard.h"
#include "event.h"
#include "blitter.h"

int __chipmem = 100 * 1024;
int __fastmem = 430 * 1024;

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static CopListT *cp;
static ConsoleT console;

#include "data/lat2-08.c"

extern uint8_t binary_data_jazzcat_electric_city_ahx_start[];
#define module binary_data_jazzcat_electric_city_ahx_start

typedef struct {
  BitmapT *bm;
  u_char *samples;
  short volume;
  int i, di;
} WaveScopeChanT;

#define TICKS_PER_FRAME (3579546L / 50)
#define WS_SAMPLES 64

static struct {
  WaveScopeChanT channel[4];
  BitmapT *spans;
  u_char multab[256*64];
} wavescope;

static void InitWaveScope(void) {
  short i, j;
  BitmapT *bm;

  for (i = 0; i < 4; i++) {
    memset(&wavescope.channel[i], 0, sizeof(WaveScopeChanT));
    wavescope.channel[i].bm = NewBitmap(64, 64, 1);
  }

  for (i = 0; i < 64; i++) {
    for (j = 0; j < 128; j++) {
      short index = (i << 8) | j;
      short x = (j * (i + 1)) >> 4;
      wavescope.multab[index] = x;
    }

    for (; j < 255; j++) {
      short index = (i << 8) | j;
      short x = ((255 - j) * (i + 1)) >> 4;
      wavescope.multab[index] = x;
    }
  }

  bm = NewBitmap(64, 32, 1);
  BlitterLineSetup(bm, 0, LINE_EOR|LINE_ONEDOT);
  BlitterLine(32, 0, 1, 31);
  BlitterLine(32, 0, 63, 31);
  BlitterFill(bm, 0);
  WaitBlitter();
  ((char *)bm->planes[0])[4] = 0x80;
  wavescope.spans = bm;
}

static void KillWaveScope(void) {
  short i;

  for (i = 0; i < 4; i++)
    DeleteBitmap(wavescope.channel[i].bm);
  DeleteBitmap(wavescope.spans);
}

static void WaveScopeUpdateChannel(short num, AhxVoiceTempT *voice) {
  WaveScopeChanT *ch = &wavescope.channel[num];
  int samplesPerFrame = 1;

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

static void WaveScopeDrawChannel(short num) {
  WaveScopeChanT *ch = &wavescope.channel[num];
  BitmapT *bm = ch->bm;
  int *dst = bm->planes[0];
  u_char *samples;
  u_char *multab;
  int i, di, volume;
  short n;

  BitmapClear(bm);

  multab = wavescope.multab;
  samples = ch->samples;
  volume = ch->volume << 8;
  i = ch->i, di = ch->di;

  for (n = 0; n < WS_SAMPLES; n++) {
    short x;

    i = swap16(i);
    x = multab[samples[i] | volume];
    i = swap16(i);

    i += di;
    if (i >= (AHX_SAMPLE_LEN << 16))
      i -= (AHX_SAMPLE_LEN << 16);

    {
      int *src = (int *)(wavescope.spans->planes[0] + (x & ~7));

      *dst++ = *src++;
      *dst++ = *src++;
    }
  }

  ch->i = i;
}

static void Load(void) {
  if (AhxInitPlayer(AHX_LOAD_WAVES_FILE, AHX_FILTERS) != 0)
    exit(10);
}

static void UnLoad(void) {
  AhxKillPlayer();
}

static __interrupt int AhxPlayerIntHandler(void) {
  /* Handle CIA Timer A interrupt. */
  if (ReadICR(ciaa) & CIAICRF_TA) {
    custom->color[0] = 0x448;
    AhxInterrupt();
    custom->color[0] = 0;
  }

  return 0;
}

INTERRUPT(AhxPlayerInterrupt, 10, AhxPlayerIntHandler, NULL);

static void AhxSetTempo(u_short tempo asm("d0")) {
  ciaa->ciatalo = tempo & 0xff;
  ciaa->ciatahi = tempo >> 8;
  ciaa->ciacra |= CIACRAF_START;
}

static void DrawFrames(void) {
  short i;

  BlitterLineSetup(screen, 0, LINE_OR|LINE_SOLID);

  for (i = 0; i < 4; i++) {
    short x1 = 8 + 72 * i - 1;
    short x2 = 72 + 72 * i + 1;
    short y1 = 96 - 1;
    short y2 = 160 + 1;

    BlitterLine(x1, y1, x2, y1);
    BlitterLine(x1, y2, x2, y2);
    BlitterLine(x1, y1, x1, y2);
    BlitterLine(x2, y1, x2, y2);
  }
}

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0xfff);
  CopEnd(cp);
  CopListActivate(cp);

  ConsoleInit(&console, &latin2, screen);

  EnableDMA(DMAF_BLITTER | DMAF_RASTER);

  ConsolePutStr(&console, 
                "Pause (SPACE) Prev (LEFT) Next (RIGHT)\n"
                "Exit (ESC)\n");
  DrawFrames();
  InitWaveScope();
  KeyboardInit();

  if (AhxInitHardware((void *)AhxSetTempo, AHX_KILL_SYSTEM) == 0)
    if (AhxInitModule(module) == 0)
      AhxInitSubSong(0, 0);

  AddIntServer(INTB_PORTS, AhxPlayerInterrupt);
}

static void Kill(void) {
  RemIntServer(INTB_PORTS, AhxPlayerInterrupt);

  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  KillWaveScope();
  AhxStopSong();
  AhxKillHardware();

  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static bool HandleEvent(void);

static void Render(void) {
  int lines = ReadLineCounter();
  short i;

  ConsoleSetCursor(&console, 0, 3);
  ConsolePrint(&console, "Position : %02d/%02d\n\n",
               Ahx.Public->Pos, Ahx.Public->Row);

  Log("Playing %d!\n", Ahx.Public->Playing);

  if (Ahx.Public->Playing) {
    WaitLine(Y(96));

    for (i = 0; i < 4; i++) {
      short x = 8 + 72 * i;
      short y = 96;

      WaveScopeUpdateChannel(i, &Ahx.Public->VoiceTemp[i]);
      WaveScopeDrawChannel(i);
      BitmapCopy(screen, x, y, wavescope.channel[i].bm);
    }
  }
  
  Log("playahx: %d\n", ReadLineCounter() - lines);

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
    Ahx.Public->Playing = ~Ahx.Public->Playing;
    custom->dmacon = (Ahx.Public->Playing ? DMAF_SETCLR : 0) | DMAF_AUDIO;
  }

  if (ev.key.code == KEY_LEFT)
    AhxPrevPattern();

  if (ev.key.code == KEY_RIGHT)
    AhxNextPattern();

  return true;
}

EFFECT(playahx, Load, UnLoad, Init, Kill, Render);
