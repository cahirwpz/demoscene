#include "std/exception.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/timer.h"
#include "system/vblank.h"

#include "startup.h"

int __nocommandline = 1;

extern EffectT Effect;

int main() {
  if (SystemCheck()) {
    LOG("Adding resources.");

    if (Effect.Load)
      Effect.Load();

    SetupTimer();
    StartEventQueue();
    StartProfiling();
    InstallVBlankIntServer();

    TRY {
      LOG("Setting up the effect.");
      Effect.Init();
      LOG("Running up main loop.");
      {
        bool loopExit = false; 

        SetVBlankCounter(0);

        do {
          int frameNumber = GetVBlankCounter();
          InputEventT event; 

          while (EventQueuePop(&event)) {
            if (event.ie_Class == IECLASS_RAWKEY)
              if (event.ie_Code & IECODE_UP_PREFIX)
                if ((event.ie_Code & ~IECODE_UP_PREFIX) == KEY_ESCAPE)
                  loopExit = true;
            if (Effect.HandleEvent && !loopExit)
              Effect.HandleEvent(&event);
          }

          Effect.Render(frameNumber);
          RenderFrameNumber(frameNumber);
          RenderFramesPerSecond(frameNumber);

          DisplaySwap();
        } while (!loopExit);
      }
      LOG("Tearing down the effect.");
      Effect.Kill();
    }
    CATCH {
      LOG("Effect crashed!");
    }

    RemoveVBlankIntServer();
    StopProfiling();
    StopEventQueue();
    KillTimer();

    if (Effect.UnLoad)
      Effect.UnLoad();
  }

  return 0;
}
