#include "std/exception.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/timer.h"
#include "system/vblank.h"

#include "startup.h"

int __nocommandline = 1;

void AcquireResources() {}
void ReleaseResources() {}
void SetupEffect() {}
void TearDownEffect() {}
void MainLoop() {}

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
      Effect.Loop();
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
