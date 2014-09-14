#include <proto/graphics.h>

#include "startup.h"
#include "file.h"
#include "hardware.h"
#include "memory.h"
#include "p61/p61.h"
#include "console.h"
#include "coplist.h"
#include "keyboard.h"
#include "interrupts.h"
#include "blitter.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static APTR module;
static BitmapT *screen;
static BitmapT *osc[4];
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

static void Load() {
  module = ReadFile("data/jazzcat-boogie_town.p61", MEMF_CHIP);
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  ITER(i, 0, 3, osc[i] = NewBitmap(64, 64, 1, FALSE));

  cp = NewCopList(100);
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopShowPlayfield(cp, screen);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }

  ConsoleInit(&console, screen, topaz8);
}

static void UnLoad() {
  FreeAutoMem(module);
  CloseFont(topaz8);
  DeleteCopList(cp);
  ITER(i, 0, 3, DeleteBitmap(osc[i]));
  DeleteBitmap(screen);
}

static __interrupt_handler void IntLevel2Handler() {
  if (custom->intreqr & INTF_PORTS) {
    /* Make sure all scratchpad registers are saved, because we call a function
     * that relies on the fact that it's caller responsibility to save them. */
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    KeyboardIntHandler();
  }

  custom->intreq = INTF_PORTS;
  custom->intreq = INTF_PORTS;
}

static inline void putpixel(UBYTE *line, WORD x) {
  bset(line + (x >> 3), ~x);
}

static __regargs void DrawOsc(BitmapT *osc, P61_OscData *data) {
  UBYTE *line = osc->planes[0];
  BYTE *sample, *end;
  WORD n = 64;

  sample = data->SamplePtr;
  end = sample + data->Count;

  if (data->WrapCount > 0)
    end -= data->WrapCount;

  for (; n > 0 && sample < end; n--, line += osc->bytesPerRow) {
    WORD x = absw(*sample++) / 8;

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
        WORD x = absw(*sample++) / 8;

        if (x > 0) {
          putpixel(line, 32 - x);
          putpixel(line, 32 + x);
        }
      }
    }
  }

  BlitterFill(osc, 0);
  WaitBlitter();
}

static void Init() {
  KeyboardInit();
  InterruptVector->IntLevel2 = IntLevel2Handler;
  custom->intena = INTF_SETCLR | INTF_PORTS;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;

  {
    WORD i;

    BlitterLineSetup(screen, 0, LINE_OR, LINE_SOLID);

    for (i = 0; i < 4; i++) {
      WORD x1 = 8 + 72 * i - 1;
      WORD x2 = 72 + 72 * i + 1;
      WORD y1 = 80 - 1;
      WORD y2 = 144 + 1;

      BlitterLineSync(x1, y1, x2, y1);
      BlitterLineSync(x1, y2, x2, y2);
      BlitterLineSync(x1, y1, x1, y2);
      BlitterLineSync(x2, y1, x2, y2);
    }
  }

  P61_Init(module, NULL, NULL);
  P61_ControlBlock.Play = 1;

  ConsolePutStr(&console, "Exit (ESC) | Pause (SPACE)\n");
}

static void Kill() {
  P61_ControlBlock.Play = 0;
  P61_End();
}

static void Render() {
  WORD i;

  ConsoleSetCursor(&console, 0, 2);
  ConsolePrint(&console, "Position : %02ld/%02ld\n\n",
               (LONG)P61_ControlBlock.Pos, (LONG)P61_ControlBlock.Row);

  ConsolePrint(&console, "Ch | Ins Cmd Vol Visu\n");
  for (i = 0; i < 4; i++) {
    P61_ChannelBlock *ch = P61_CHANNEL(i);
    /* Command upper 4 bits store instrument number. */
    WORD cmd = ((ch->Command & 15) << 8) | ch->Info;
    WORD ins = (ch->Command >> 4) | ((ch->SN_Note & 1) << 4);
    ConsolePrint(&console, " %ld |  %02lx %03lx  %02ld %04lx\n",
                 (LONG)i, (LONG)ins, (LONG)cmd,
                 (LONG)(ch->Volume), (LONG)P61_visuctr[i]);
  }


  if (P61_ControlBlock.Play) {
    WaitLine(Y(96));

    for (i = 0; i < 4; i++) {
      P61_OscData data;

      BlitterClear(osc[i], 0);
      WaitBlitter();

      if (P61_Osc(P61_CHANNEL(i), &data))
        DrawOsc(osc[i], &data);

      BlitterLineSetup(osc[i], 0, LINE_OR, LINE_SOLID);
      BlitterLine(32, 0, 32, 64);
      WaitBlitter();
    }

    for (i = 0; i < 4; i++) {
      WORD x = 8 + 72 * i;
      WORD y = 80;
      BlitterCopySync(screen, 0, x, y, osc[i], 0);
    }
  }

  WaitVBlank();
}

static BOOL HandleEvent() {
  KeyEventT event;

  if (!GetKeyEvent(&event))
    return TRUE;

  if (event.modifier & MOD_PRESSED)
    return TRUE;

  if (event.code == KEY_ESCAPE)
    return FALSE;

  if (event.code == KEY_SPACE) {
    P61_ControlBlock.Play ^= 1;
    if (P61_ControlBlock.Play)
      custom->dmacon = DMAF_SETCLR | DMAF_AUDIO;
    else
      custom->dmacon = DMAF_AUDIO;
  }

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, HandleEvent };
