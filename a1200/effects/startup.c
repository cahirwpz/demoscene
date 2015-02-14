#include <clib/exec_protos.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/exception.h"
#include "std/resource.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/timer.h"
#include "system/vblank.h"

#include "startup.h"

int __nocommandline = 1;

extern void MainLoop();
extern bool SetupDisplay();
extern void TearDownEffect();
extern void SetupEffect();
extern void AddInitialResources();

int main() {
  if (SystemCheck()) {
    LOG("Adding resources.");
    StartResourceManager();

    TRY {
      AddInitialResources();
    }
    CATCH {
      return 1;
    }

    SetupTimer();
    StartEventQueue();

    if (SetupDisplay()) {
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
      KillDisplay();
    }

    KillTimer();
    StopEventQueue();
    StopResourceManager();
  }

  return 0;
}
