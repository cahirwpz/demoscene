#include <clib/exec_protos.h>
#include <inline/exec_protos.h>
#include <proto/exec.h>

#include "system/input.h"
#include "system/resource.h"
#include "system/vblank.h"

struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;

extern void SetupDisplayAndRun();

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 40))) {
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 40))) {
      if (ResourcesAlloc()) {
        if (ResourcesInit()) {
          if (InitEventHandler()) {
            InstallVBlankIntServer();
            SetupDisplayAndRun();
            RemoveVBlankIntServer();
            KillEventHandler();
          }
        }
      }
      ResourcesFree();

      CloseLibrary((struct Library *)GfxBase);
    }
    CloseLibrary((struct Library *)DOSBase);
  }

  return 0;
}
