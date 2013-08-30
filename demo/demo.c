#include <clib/exec_protos.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/exception.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

#include "config.h"
#include "demo.h"

bool ExitDemo = false;

static void DoTimeSlice(TimeSliceT *slice, int thisFrame, int globalLastFrame) {
  while (slice->data.func) {
    bool invoke = false;

    if (slice->type < 0) {
      /* Recurse? */
      if ((slice->start <= thisFrame) && (thisFrame < slice->end))
        DoTimeSlice(slice->data.slice,
                    thisFrame - slice->start,
                    globalLastFrame - slice->start);
    } else {
      int step = slice->type;

      switch (step) {
        case 0:
          /* Do it only once. */
          invoke = (slice->start >= globalLastFrame) && (slice->start <= thisFrame);
          break;

        case 1:
          /* Do it every frame. */
          invoke = (slice->start <= thisFrame) && (thisFrame < slice->end);
          break;

        default:
          /* Do it every n-th frame. */
          if ((slice->start <= thisFrame) && (thisFrame < slice->end)) {
            if (slice->last == -1) {
              slice->last = slice->start;
              invoke = true;
            } else {
              if (thisFrame - slice->last >= step) {
                slice->last = thisFrame - ((thisFrame - slice->start) % step);
                invoke = true;
              }
            }
          }
          break;
      }

      if (invoke) {
        FrameT frame = { slice->start, slice->end - 1, thisFrame - slice->start };

        slice->data.func(&frame);
      }
    }

    slice++;
  }
}

void RunEffects(TimeSliceT *slices, int thisFrame) {
  static int globalLastFrame = 0;

  DoTimeSlice(slices, thisFrame, globalLastFrame);

  globalLastFrame = thisFrame + 1;
}

int main() {
  if (SystemCheck()) {
    JsonNodeT *config;

    if ((config = ReadConfig())) {
      static bool ready = true;

      StartResourceManager();
      StartEventQueue();

      TRY {
        SetupDemo();
        LoadResources(config);
        SetupResources();
      }
      CATCH {
        ready = false;
      }

      if (ready) {
        int frameNumber;

        InstallVBlankIntServer();
        SetVBlankCounter(0);

        do {
          frameNumber = GetVBlankCounter();

          RunEffects(TheDemo, frameNumber);
          DisplaySwap();
          HandleEvents(frameNumber);
        } while (!ExitDemo);

        RemoveVBlankIntServer();

        KillDemo();
      }

      KillDisplay();
      StopEventQueue();
      StopResourceManager();

      MemUnref(config);
    }
  }

  return 0;
}
