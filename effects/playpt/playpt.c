#include <effect.h>
#include <blitter.h>
#include <ptplayer.h>
#include <console.h>
#include <copper.h>
#include <system/event.h>
#include <system/keyboard.h>
#include <system/memory.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

#include "data/lat2-08.c"

extern u_char Module[];
extern u_char Samples[];
#ifdef DELTA
extern u_char SamplesSize[];
#endif

static BitmapT *screen;
static CopListT *cp;
static ConsoleT console;

/* Extra variables to enhance replayer functionality */
static bool stopped = true;

#ifdef AKLANG
extern u_int AK_Progress;
void AK_Generate(void *TmpBuf asm("a1"));
#endif

#ifdef DELTA
static void DecodeSamples(u_char *smp, int size) {
  u_char data = *smp++;
  short n = (size + 7) / 8 - 1;
  short k = size & 7;

  Log("[Load] Decoding delta samples (%d bytes)\n", size);

  switch (k) {
  case 0: do { data += *smp; *smp++ = data;
  case 7:      data += *smp; *smp++ = data;
  case 6:      data += *smp; *smp++ = data;
  case 5:      data += *smp; *smp++ = data;
  case 4:      data += *smp; *smp++ = data;
  case 3:      data += *smp; *smp++ = data;
  case 2:      data += *smp; *smp++ = data;
  case 1:      data += *smp; *smp++ = data;
          } while (--n != -1);
  }
}
#endif

static void Load(void) {
#ifdef AKLANG
  void *TmpBuf = MemAlloc(AKLANG_BUFLEN, MEMF_PUBLIC);
  Log("[Load] Generating Amiga Klang samples (%d bytes)!\n", AKLANG_BUFLEN);
  AK_Generate(TmpBuf);
  MemFree(TmpBuf);
#endif
#ifdef DELTA
  DecodeSamples(Samples, (int)SamplesSize);
#endif
}

static void Init(void) {
  KeyboardInit();

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, 0x000);
  SetColor(1, 0xfff);

  cp = NewCopList(100);
  CopSetupBitplanes(cp, screen, DEPTH);
  CopListFinish(cp);

  ConsoleInit(&console, &latin2, screen);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  ConsoleSetCursor(&console, 0, 0);
  ConsolePutStr(&console, "Initializing Protracker replayer...!\n");

  PtInstallCIA();
  PtInit(Module, Samples, 1);
  PtEnable = 1;

  ConsoleSetCursor(&console, 0, 0);
  ConsolePutStr(&console, 
                "Pause (SPACE) -10s (LEFT) +10s (RIGHT)\n"
                "Exit (ESC)\n");

  stopped = false;
}

static void Kill(void) {
  PtEnd();
  PtRemoveCIA();

  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_AUDIO);

  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static bool HandleEvent(void);

static const char hex[16] = "0123456789ABCDEF";

static inline void NumToHex(char *str, u_int n, short l) {
  do {
    str[--l] = hex[n & 15];
    n >>= 4;
    if (n == 0)
      break;
  } while (l > 0);
}

static void NoteToHex(char *str, u_short note, u_short cmd) {
  /* instrument number */
  NumToHex(&str[0], ((note >> 8) & 0xf0) | ((cmd >> 12) & 15), 2);
  /* note period */
  NumToHex(&str[2], note & 0xfff, 3);
  /* effect command */
  NumToHex(&str[5], cmd & 0xfff, 3);
}

static void Render(void) {
  PtModule *mod = PtData.mt_mod;
  short songPos = PtData.mt_SongPos;
  short pattNum = mod->order[songPos];
  short pattPos = PtData.mt_PatternPos >> 4;
  u_int *pattern = (void *)mod->pattern[pattNum];
  short i;

  ConsoleSetCursor(&console, 0, 3);
  ConsolePrint(&console, "Playing \"%s\"\n\n", mod->name);
  ConsolePrint(&console, "Song position: %d -> %d\n\n", songPos, pattNum);

  for (i = pattPos - 4; i < pattPos + 4; i++) {
    static char buf[41];

    memset(buf, ' ', sizeof(buf));
    buf[40] = '\0';

    if ((i >= 0) && (i < 64)) {
      u_short *row = (void *)&pattern[4 * i];
      NumToHex(&buf[0], i, 2);
      buf[2] = ':';
      NoteToHex(&buf[3], row[0], row[1]);
      buf[11] = '|';
      NoteToHex(&buf[12], row[2], row[3]);
      buf[20] = '|';
      NoteToHex(&buf[21], row[4], row[5]);
      buf[29] = '|';
      NoteToHex(&buf[30], row[6], row[7]);
      buf[38] = '|';
    }

    ConsolePutStr(&console, buf);
  }

  ConsolePutChar(&console, '\n');

  for (i = 0; i < 4; i++) {
    PtChannel *chan = &PtData.mt_chan[i];
    ConsolePrint(&console, "CH%d: %08x %04x %08x %04x\n",
                 i, (u_int)chan->n_start, chan->n_length,
                 (u_int)chan->n_loopstart, chan->n_replen);
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
    PtEnable ^= 1;
    if (!PtEnable)
      DisableDMA(DMAF_AUDIO);
  }

  if (ev.key.code == KEY_LEFT) {
  }

  if (ev.key.code == KEY_RIGHT) {
  }

  return true;
}

EFFECT(PlayProtracker, Load, NULL, Init, Kill, Render, NULL);
