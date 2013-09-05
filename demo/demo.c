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

/* enable rewinding and fast-forward keys */
static bool DemoTimeKeys = false;
/* show the number of current frame */
static bool DemoShowFrame = false;
/* first frame of the demo to be played */
static int DemoFirstFrame = 0;
/* last frame of the demo to be played */
static int DemoLastFrame = -1;
/* should the range of frames above be played in loop ? */
static bool DemoLoop = false;

/*
 * Handle events during the demo.
 */
static bool HandleEvents(int frameNumber) {
  static InputEventT event; 
  static int counter = 1;

  while (EventQueuePop(&event)) {
    if (event.ie_Class == IECLASS_RAWKEY) {
      if (event.ie_Code & IECODE_UP_PREFIX) {
        if (DemoTimeKeys) {
          bool timeUpdated = false;

          switch (event.ie_Code & ~IECODE_UP_PREFIX) {
            case KEY_UP:
              ChangeVBlankCounter(-10 * FRAMERATE);
              timeUpdated = true;
              break;

            case KEY_DOWN:
              ChangeVBlankCounter(10 * FRAMERATE);
              timeUpdated = true;
              break;

            case KEY_LEFT:
              ChangeVBlankCounter(-FRAMERATE);
              timeUpdated = true;
              break;

            case KEY_RIGHT:
              ChangeVBlankCounter(FRAMERATE);
              timeUpdated = true;
              break;

            case KEY_SPACE:
              LOG("Event %d at %.2f (frame %d).",
                  counter++, (float)frameNumber / FRAMERATE, frameNumber);
              break;

            default:
              break;
          }

          if (timeUpdated)
            DemoUpdateTime(frameNumber, GetVBlankCounter());
        }

        if ((event.ie_Code & ~IECODE_UP_PREFIX) == KEY_ESCAPE)
          return true;
      }
    }
  }

  return false;
}

static void ParsePlaybackInfo() {
  struct RDArgs *rdargs;
  struct {
    int *first;
    int *last;
    int loop;
  } args = { NULL, NULL, 0 };

  DemoLastFrame = DemoEndFrame;

  if ((rdargs = ReadArgs("FIRST/N,LAST/N,LOOP/S", (LONG *)&args, NULL))) {
    if (args.first) {
      if (*args.first > 0 && *args.first < DemoEndFrame) {
        DemoFirstFrame = *args.first;
        LOG("Changed first frame to %d.", DemoFirstFrame);
      }
    }

    if (args.last) {
      if (*args.last > 0 && *args.last < DemoEndFrame) {
        DemoLastFrame = *args.last;
        LOG("Changed last frame to %d.", DemoLastFrame);
      }
    }

    if (DemoFirstFrame >= DemoLastFrame) {
      LOG("Wrong frame range specification. Reverting to original values.");
      DemoFirstFrame = 0;
      DemoLastFrame = DemoEndFrame;
    }

    if (args.loop) {
      LOG("Will play demo in loop.");
      DemoLoop = true;
    }

    FreeArgs(rdargs);
  }
}

int main() {
  if (SystemCheck()) {
    if (ReadConfig()) {
      DemoTimeKeys = JsonQueryBoolean(DemoConfig, "flags/time-keys");
      DemoShowFrame = JsonQueryBoolean(DemoConfig, "flags/show-frame");

      if (InitAudio()) {
        static bool ready = true;
        static TimeSliceT *demo;

        StartResourceManager();
        StartEventQueue();

        TRY {
          demo = LoadTimeline();

          if (!LoadDemo())
            PANIC("Loading demo failed.");

          LoadResources();
          SetupResources();
        }
        CATCH {
          ready = false;
        }

        if (ready) {
          int frameNumber;

          ParsePlaybackInfo();
          BeginDemo();
          InstallVBlankIntServer();
          SetVBlankCounter(DemoFirstFrame);

          while (true) {
            frameNumber = GetVBlankCounter();

            if (frameNumber > DemoLastFrame) {
              if (!DemoLoop)
                break;

              SetVBlankCounter(DemoFirstFrame);
              DemoUpdateTime(frameNumber, DemoFirstFrame);
            }

            DoTimeSlice(demo, frameNumber);

            if (DemoShowFrame) {
              RenderFrameNumber(frameNumber);
              RenderFramesPerSecond(frameNumber);
            }

            DisplaySwap();

            if (HandleEvents(frameNumber))
              break;
          }

          RemoveVBlankIntServer();

          KillDemo();
        }

        MemUnref(demo);

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
