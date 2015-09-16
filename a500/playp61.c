#include <proto/graphics.h>

#include "startup.h"
#include "io.h"
#include "hardware.h"
#include "memory.h"
#include "p61/p61.h"
#include "console.h"
#include "coplist.h"
#include "keyboard.h"
#include "bltop.h"

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
  module = LoadFile("data/jazzcat-sunglasses_at_night.p61", MEMF_CHIP);
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  ITER(i, 0, 3, osc[i] = NewBitmap(64, 64, 1));

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
}

static void UnLoad() {
  MemFree(module);
  CloseFont(topaz8);
  DeleteCopList(cp);
  ITER(i, 0, 3, DeleteBitmap(osc[i]));
  DeleteBitmap(screen);
}

static void VBlankHandler() {
  if (custom->intreqr & INTF_VERTB)
    P61_Music();
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
}

static void Init() {
  KeyboardInit();

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  {
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

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
  custom->intena = INTF_SETCLR | INTF_VERTB;

  P61_Init(module, NULL, NULL);
  P61_ControlBlock.Play = 1;

  ConsolePutStr(&console, 
                "Pause (SPACE) Prev (LEFT) Next (RIGHT)\n"
                "Exit (ESC)\n");
}

static void Kill() {
  P61_ControlBlock.Play = 0;
  P61_End();

  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;
  custom->intena = INTF_VERTB;
}

static void Render() {
  WORD i;

  ConsoleSetCursor(&console, 0, 3);
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

      BitmapClear(osc[i], 1);

      if (P61_Osc(P61_CHANNEL(i), &data))
        DrawOsc(osc[i], &data);

      BlitterLineSetup(osc[i], 0, LINE_OR, LINE_SOLID);
      BlitterLine(32, 0, 32, 64);
    }

    for (i = 0; i < 4; i++) {
      WORD x = 8 + 72 * i;
      WORD y = 96;
      BitmapCopy(screen, x, y, osc[i]);
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

  if (event.code == KEY_LEFT) {
    BYTE newPos = P61_ControlBlock.Pos - 2;
    if (newPos < 0)
      newPos = 0;
    P61_SetPosition(newPos);
  }

  if (event.code == KEY_RIGHT)
    P61_SetPosition(P61_ControlBlock.Pos);

  return TRUE;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, VBlankHandler, HandleEvent };
