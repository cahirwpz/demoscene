#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "std/memory.h"
#include "std/debug.h"
#include "std/exception.h"
#include "std/resource.h"
#include "system/audio.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"
#include "tools/frame.h"

#include "config.h"
#include "demo.h"
#include "timeline.h"

int __nocommandline = 1;

bool ExitDemo = false;

/* enable rewinding and fast-forward keys */
static bool DemoTimeKeys = false;
/* show the number of current frame */
static bool DemoShowFrame = false;

/*
 * Handle events during the demo.
 */
static void HandleEvents(int frameNumber) {
  static InputEventT event; 
  static int counter = 1;

  while (EventQueuePop(&event)) {
    if (event.ie_Class == IECLASS_RAWKEY) {
      if (event.ie_Code & IECODE_UP_PREFIX) {
        if (DemoTimeKeys) {
          bool posChanged = false;

          switch (event.ie_Code & ~IECODE_UP_PREFIX) {
            case KEY_UP:
              ChangeVBlankCounter(-10 * FRAMERATE);
              posChanged = true;
              break;

            case KEY_DOWN:
              ChangeVBlankCounter(10 * FRAMERATE);
              posChanged = true;
              break;

            case KEY_LEFT:
              ChangeVBlankCounter(-FRAMERATE);
              posChanged = true;
              break;

            case KEY_RIGHT:
              ChangeVBlankCounter(FRAMERATE);
              posChanged = true;
              break;

            case KEY_SPACE:
              LOG("Event %d at %.2f (frame %d).",
                  counter++, (float)frameNumber / FRAMERATE, frameNumber);
              break;

            default:
              break;
          }

          if (posChanged)
            DemoUpdateTime(frameNumber, GetVBlankCounter());
        }

        if ((event.ie_Code & ~IECODE_UP_PREFIX) == KEY_ESCAPE)
          ExitDemo = TRUE;
      }
    }
  }
}

static void ParseArgs() {
  struct RDArgs *rdargs;
  struct {
    char *start;
  } args;

  if ((rdargs = ReadArgs("START/K", (LONG *)&args, NULL))) {
    FreeArgs(rdargs);
  }
}

int main() {
  ParseArgs();

  if (SystemCheck()) {
    if (ReadConfig()) {
      DemoTimeKeys = JsonQueryBoolean(DemoConfig, "flags/time-keys");
      DemoShowFrame = JsonQueryBoolean(DemoConfig, "flags/show-frame");

      if (InitAudio()) {
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

            if (DemoShowFrame) {
              RenderFrameNumber(frameNumber);
              RenderFramesPerSecond(frameNumber);
            }

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
    }
  }

  return 0;
}
