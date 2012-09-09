#include <clib/exec_protos.h>
#include <inline/exec_protos.h>
#include <proto/exec.h>

#include "std/resource.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;

extern void MainLoop();
extern void TearDownDisplay();
extern void TearDownEffect();
extern struct ViewPort *SetupDisplay();
extern void SetupEffect();

int main() {
  DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 40);
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 40);

  if (DosBase && GfxBase) {
    if (ResourcesAlloc() && ResourcesInit()) {
      if (InitEventHandler()) {
        struct View *view;
        struct ViewPort *viewPort;

        if ((view = NewView())) {
          SaveOrigView();

          if ((viewPort = SetupDisplay())) {
            ApplyView(view, viewPort);

            InstallVBlankIntServer();
            SetupEffect();
            MainLoop();
            TearDownEffect();
            RemoveVBlankIntServer();

            RestoreOrigView();
            DeleteView(view);
            TearDownDisplay();
          }
        }
        KillEventHandler();
      }
    }

    ResourcesFree();
  }

  CloseLibrary((struct Library *)GfxBase);
  CloseLibrary((struct Library *)DOSBase);

  return 0;
}
