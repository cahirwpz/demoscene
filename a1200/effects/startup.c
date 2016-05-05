#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include "std/exception.h"
#include "system/display.h"
#include "system/input.h"

#include "startup.h"

int __nocommandline = 1;

extern EffectT Effect;

static bool SystemCheck() {
  bool kickv40 = SysBase->LibNode.lib_Version >= 40;
  bool chipaga = GfxBase->ChipRevBits0 & (GFXF_AA_ALICE|GFXF_AA_LISA);
  bool cpu68060 = SysBase->AttnFlags & AFF_68060;
  bool fpu68882 = SysBase->AttnFlags & AFF_68882;

  Printf("System check:\r\n");
  Printf(" - Kickstart v40 : %s\r\n", (ULONG)(kickv40 ? "yes" : "no"));
  Printf(" - ChipSet AGA : %s\r\n", (ULONG)(chipaga ? "yes" : "no"));
  Printf(" - CPU 68060 : %s\r\n", (ULONG)(cpu68060 ? "yes" : "no"));
  Printf(" - FPU 68882 : %s\r\n", (ULONG)(fpu68882 ? "yes" : "no"));

  return (kickv40 && cpu68060 && fpu68882 && chipaga);
}

int main() {
  if (SystemCheck()) {
    LOG("Adding resources.");

    if (Effect.Load)
      Effect.Load();

    StartEventQueue();
    StartProfiling();
    SetFrameCounter(0);
    TRY {
      BOOL loopExit = false;

      LOG("Setting up the effect.");
      Effect.Init();
      LOG("Running up main loop.");

      do {
        InputEventT event; 
        int32_t frameNumber;

        while (EventQueuePop(&event)) {
          if (event.ie_Class == IECLASS_RAWKEY)
            if (event.ie_Code & IECODE_UP_PREFIX)
              if ((event.ie_Code & ~IECODE_UP_PREFIX) == KEY_ESCAPE)
                loopExit = true;
          if (Effect.HandleEvent && !loopExit)
            Effect.HandleEvent(&event);
        }

        frameNumber = ReadFrameCounter();
        Effect.Render(frameNumber);
        RenderFrameNumber(frameNumber);
        RenderFramesPerSecond(frameNumber);

        DisplaySwap();
      } while (!loopExit);

      LOG("Tearing down the effect.");
      Effect.Kill();
    }
    CATCH {
      LOG("Effect crashed!");
    }
    StopProfiling();
    StopEventQueue();

    if (Effect.UnLoad)
      Effect.UnLoad();
  }

  return 0;
}
