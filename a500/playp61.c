#include <proto/graphics.h>

#include "file.h"
#include "hardware.h"
#include "memory.h"
#include "p61/p61.h"
#include "console.h"
#include "coplist.h"
#include "keyboard.h"
#include "interrupts.h"
#include "blitter.h"

static APTR module;
static BitmapT *screen;
static BitmapT *osc[4];
static CopListT *cp;
static TextFontT *topaz8;
static ConsoleT console;

void Load() {
  module = ReadFile("data/tempest-acidjazzed_evening.p61", MEMF_CHIP);
  screen = NewBitmap(320, 256, 1, FALSE);
  ITER(i, 0, 3, osc[i] = NewBitmap(64, 64, 1, FALSE));

  cp = NewCopList(100);
  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  {
    struct TextAttr textattr = { "topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    topaz8 = OpenFont(&textattr);
  }

  ConsoleInit(&console, screen, topaz8);
}

void Kill() {
  FreeAutoMem(module);
  CloseFont(topaz8);
  DeleteCopList(cp);
  ITER(i, 0, 3, DeleteBitmap(osc[i]));
  DeleteBitmap(screen);
}

__interrupt_handler void IntLevel2Handler() {
  if (custom->intreqr & INTF_PORTS) {
    /* Make sure all scratchpad registers are saved, because we call a function
     * that relies on the fact that it's caller responsibility to save them. */
    asm volatile("" ::: "d0", "d1", "a0", "a1");
    KeyboardIntHandler();
  }

  custom->intreq = INTF_PORTS;
  custom->intreq = INTF_PORTS;
}

static BYTE SampleNum[4];
static WORD SampleWait[4] = {-1, -1, -1, -1};

void Main() {
  KeyboardInit();
  InterruptVector->IntLevel2 = IntLevel2Handler;
  custom->intena = INTF_SETCLR | INTF_PORTS;

  P61_Init(module, NULL, NULL);
  P61_ControlBlock.Play = 1;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;

  ConsolePutStr(&console,
                "ESC   : exit player\n"
                "SPACE : pause\n");

  {
    WORD i;

    BlitterLineSetup(screen, 0, LINE_OR, LINE_SOLID);

    for (i = 0; i < 4; i++) {
      WORD x1 = 8 + 72 * i - 1;
      WORD x2 = 72 + 72 * i + 1;
      WORD y1 = 64 - 1;
      WORD y2 = 128 + 1;

      BlitterLineSync(x1, y1, x2, y1);
      BlitterLineSync(x1, y2, x2, y2);
      BlitterLineSync(x1, y1, x1, y2);
      BlitterLineSync(x2, y1, x2, y2);
    }
  }

  while (1) {
    KeyEventT event;
    WORD i, j;

    if (GetKeyEvent(&event)) {
      if (event.modifier & MOD_PRESSED)
        continue;

      if (event.code == KEY_ESCAPE)
        break;

#if 0
      if (event.code == KEY_RIGHT)
        P61_SetPosition(P61_ControlBlock.Pos + 1);

      if (event.code == KEY_RIGHT)
        P61_SetPosition(P61_ControlBlock.Pos - 1);
#endif
    }

    ConsoleSetCursor(&console, 0, 3);
    ConsolePrint(&console, "Position : %02ld/%02ld\n",
                 (LONG)P61_ControlBlock.Pos, (LONG)P61_ControlBlock.Row);

    ConsolePutStr(&console, "Samples  : ");
    for (i = 0; i < 4; i++) {
      BYTE num = P61_CHANNEL(i)->Sample - P61_Samples;
      if (num != SampleNum[i]) {
        ConsolePrint(&console, "%02lx ", (LONG)num);
        SampleNum[i] = num;
        SampleWait[i] = 8;
      } else {
        if (--SampleWait[i] < 0)
          ConsolePutStr(&console, "-- ");
        else
          ConsolePrint(&console, "%02lx ", (LONG)num);
      }
    }
    ConsolePutStr(&console, "\n");

    for (i = 0; i < 4; i++) {
      P61_OscData data;

      BlitterClear(osc[i], 0);
      WaitBlitter();

      if (P61_Osc(P61_CHANNEL(i), &data)) {
        if (data.WrapCount <= 0) {
          UBYTE *pixel = osc[i]->planes[0];
          for (j = 0; j < min(data.Count, 64); j++, pixel += 64 / 8) {
            WORD x = abs(data.SamplePtr[j]) / 8;

            if (x > 31)
              x = 31;

            {
              WORD x1 = 32 - x;
              WORD x2 = 32 + x;
              bset(pixel + (x1 >> 3), ~x1);
              bset(pixel + (x2 >> 3), ~x2);
            }
          }

          BlitterFill(osc[i], 0);
          WaitBlitter();
        }
      }
    }

    WaitLine(128);
    for (i = 0; i < 4; i++) {
      WORD x1 = 8 + 72 * i;
      WORD y1 = 64;
      BlitterCopySync(screen, 0, x1, y1, osc[i], 0);
    }

    WaitVBlank();
  }

  P61_ControlBlock.Play = 0;
  P61_End();
}
