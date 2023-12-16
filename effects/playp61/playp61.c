#include <effect.h>
#include <blitter.h>
#include <console.h>
#include <copper.h>
#include <p61.h>
#include <system/event.h>
#include <system/keyboard.h>
#include <system/memory.h>
#include <system/timer.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

#include "data/drdos8x8.c"

extern u_char binary_data_jazzcat_sunglasses_at_night_p61_start[];
#define module binary_data_jazzcat_sunglasses_at_night_p61_start

static BitmapT *screen;
static BitmapT *osc[4];
static CopListT *cp;
static ConsoleT console;
static CIATimerT *p61tmr;

static inline void putpixel(u_char *line, short x) {
  bset(line + (x >> 3), ~x);
}

static void DrawOsc(BitmapT *osc, P61_OscData *data) {
  u_char *line = osc->planes[0];
  char *sample, *end;
  short n = 64;

  sample = data->SamplePtr;
  end = sample + data->Count;

  if (data->WrapCount > 0)
    end -= data->WrapCount;

  for (; n > 0 && sample < end; n--, line += osc->bytesPerRow) {
    short x = absw(*sample++) / 8;

    if (x > 0) {
      putpixel(line, 32 - x);
      putpixel(line, 32 + x);
    }
  }

  if (data->WrapCount > 0 && data->LoopEndPtr) {
    end = data->LoopEndPtr;

    while (n > 0) {
      sample = data->LoopStartPtr;

      for (; n > 0 && sample < end; n--, line += osc->bytesPerRow) {
        short x = absw(*sample++) / 8;

        if (x > 0) {
          putpixel(line, 32 - x);
          putpixel(line, 32 + x);
        }
      }
    }
  }

  BlitterFill(osc, 0);
}

static void Init(void) {
  KeyboardInit();

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  ITER(i, 0, 3, osc[i] = NewBitmap(64, 64, 1, BM_CLEAR));

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, 0x000);
  SetColor(1, 0xfff);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupBitplanes(cp, screen, DEPTH);
  CopEnd(cp);

  ConsoleInit(&console, &drdos8x8, screen);

  EnableDMA(DMAF_BLITTER);

  {
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

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);

  /* This timer launches P61 sound DMA reprogramming. */
  p61tmr = AcquireTimer(TIMER_CIAB_B);
  Assert(p61tmr != NULL);

  P61_Init(module, NULL, NULL);
  P61_ControlBlock.Play = 1;

  ConsolePutStr(&console, 
                "Pause (SPACE) Prev (LEFT) Next (RIGHT)\n"
                "Exit (ESC)\n");
}

static void Kill(void) {
  P61_ControlBlock.Play = 0;
  P61_End();

  ReleaseTimer(p61tmr);

  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp);
  ITER(i, 0, 3, DeleteBitmap(osc[i]));
  DeleteBitmap(screen);
}

static bool HandleEvent(void);

static void Render(void) {
  short i;

  ConsoleSetCursor(&console, 0, 3);
  ConsolePrint(&console, "Position : %02d/%02d\n\n",
               P61_ControlBlock.Pos, P61_ControlBlock.Row);

  ConsolePrint(&console, "Ch | Ins Cmd Vol Visu\n");
  for (i = 0; i < 4; i++) {
    P61_ChannelBlock *ch = P61_CHANNEL(i);
    /* Command upper 4 bits store instrument number. */
    short cmd = ((ch->Command & 15) << 8) | ch->Info;
    short ins = (ch->Command >> 4) | ((ch->SN_Note & 1) << 4);
    ConsolePrint(&console, " %d |  %02x %03x  %02d %04x\n",
                 i, ins, cmd, ch->Volume, P61_visuctr[i]);
  }


  if (P61_ControlBlock.Play) {
    WaitLine(Y(96));

    for (i = 0; i < 4; i++) {
      P61_OscData data;

      BitmapClear(osc[i]);

      if (P61_Osc(P61_CHANNEL(i), &data))
        DrawOsc(osc[i], &data);

      BlitterLineSetup(osc[i], 0, LINE_OR|LINE_SOLID);
      BlitterLine(32, 0, 32, 64);
    }

    for (i = 0; i < 4; i++) {
      short x = 8 + 72 * i;
      short y = 96;
      BitmapCopy(screen, x, y, osc[i]);
    }
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
    P61_ControlBlock.Play ^= 1;
    if (P61_ControlBlock.Play)
      EnableDMA(DMAF_AUDIO);
    else
      DisableDMA(DMAF_AUDIO);
  }

  if (ev.key.code == KEY_LEFT) {
    char newPos = P61_ControlBlock.Pos - 2;
    if (newPos < 0)
      newPos = 0;
    P61_SetPosition(newPos);
  }

  if (ev.key.code == KEY_RIGHT)
    P61_SetPosition(P61_ControlBlock.Pos);

  return true;
}

EFFECT(PlayP61, NULL, NULL, Init, Kill, Render, P61_Music);
