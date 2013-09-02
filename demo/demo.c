#include <clib/exec_protos.h>
#include <proto/exec.h>

#include "std/memory.h"
#include "std/debug.h"
#include "std/exception.h"
#include "std/resource.h"
#include "system/audio.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

#include "config.h"
#include "demo.h"
#include "timeline.h"

bool ExitDemo = false;

int main() {
  if (SystemCheck() && InitAudio() && ReadConfig()) {
    static bool ready = true;
    static TimeSliceT *demo;
    static float beat;

    StartResourceManager();
    StartEventQueue();

    TRY {
      demo = LoadTimeline();
      beat = GetBeatLength();

      if (!LoadDemo())
        PANIC("Loading demo failed.");

      LoadResources();
      SetupResources();
    }
    CATCH {
      ready = false;
    }

    if (ready) {
      BeginDemo();

      InstallVBlankIntServer();
      SetVBlankCounter(0);

      do {
        int frameNumber = GetVBlankCounter();
        FrameT frame = { .beat = beat };

        DoTimeSlice(demo, &frame, frameNumber);
        DisplaySwap();
        HandleEvents(frameNumber);
      } while (!ExitDemo);

      RemoveVBlankIntServer();

      KillDemo();
    }

    KillAudio();
    KillDisplay();
    StopEventQueue();
    StopResourceManager();

    MemUnref(DemoConfig);
  }

  return 0;
}
