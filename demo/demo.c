#include <clib/exec_protos.h>
#include <proto/exec.h>

#include "std/resource.h"
#include "system/check.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

#include "demo.h"

struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;

void RunEffects(TimeSliceT *slice, int thisFrame) {
  static int globalLastFrame = 0;

  while (slice->func) {
    bool invoke = FALSE;

    switch (slice->step) {
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
            invoke = TRUE;
          } else {
            if (thisFrame - slice->last >= slice->step) {
              slice->last = thisFrame - ((thisFrame - slice->start) % slice->step);
              invoke = TRUE;
            }
          }
        }
        break;
    }

    if (invoke)
      slice->func(thisFrame - slice->start);

    slice++;
  }

  globalLastFrame = thisFrame + 1;
}

int main() {
  DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 39);
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);
  IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);

  if (DOSBase && GfxBase && IntuitionBase && SystemCheck()) {
    StartResourceManager();
    StartEventQueue();

    if (SetupDemo()) {
      int frameNumber;

      InstallVBlankIntServer();
      SetVBlankCounter(0);

      do {
        frameNumber = GetVBlankCounter();

        RunEffects(Timeline, frameNumber);
        DisplaySwap();
      } while (!HandleEvents(frameNumber));

      KillDemo();
      RemoveVBlankIntServer();
      KillDisplay();
    }

    StopEventQueue();
    StopResourceManager();
  }

  CloseLibrary((struct Library *)IntuitionBase);
  CloseLibrary((struct Library *)GfxBase);
  CloseLibrary((struct Library *)DOSBase);

  return 0;
}
