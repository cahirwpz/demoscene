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

void RunEffects(TimeSliceT *slice, int frameNumber) {
  for (; slice->func; slice++) {
    if (frameNumber < slice->start)
      continue;
    if ((slice->end > 0) && (slice->end >= frameNumber))
      continue;
    if ((frameNumber % slice->step) != 0)
      continue;

    slice->func(frameNumber);
  }
}

int main() {
  DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 39);
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);
  IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);

  if (DOSBase && GfxBase && IntuitionBase && SystemCheck()) {
    StartResourceManager();
    AddDemoResources();
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
