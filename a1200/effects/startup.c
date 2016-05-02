#include <clib/exec_protos.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/exception.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/timer.h"
#include "system/vblank.h"
#include "tools/profiling.h"

#include "startup.h"

int __nocommandline = 1;

extern void AcquireResources();
extern void ReleaseResources();
extern void SetupEffect();
extern void TearDownEffect();
extern void MainLoop();

bool SetupDisplay() {
  return false;
}

int main() {
  if (SystemCheck()) {
    LOG("Adding resources.");

    AcquireResources();

    SetupTimer();
    StartEventQueue();
    StartProfiling();
    InstallVBlankIntServer();

    TRY {
      LOG("Setting up the effect.");
      SetupEffect();
      LOG("Running up main loop.");
      MainLoop();
      LOG("Tearing down the effect.");
      TearDownEffect();
    }
    CATCH {
      LOG("Effect crashed!");
    }

    RemoveVBlankIntServer();
    StopProfiling();
    StopEventQueue();
    KillTimer();

    ReleaseResources();
  }

  return 0;
}
