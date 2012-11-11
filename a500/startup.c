#include <exec/types.h>

#include <inline/exec.h>
#include <inline/graphics.h>
#include <inline/dos.h> 

#include "startup.h"
#include "hardware.h"

extern void Main();

int __nocommandline = 1;
int __initlibraries = 0;

struct DosLibrary *DOSBase = NULL;
struct GfxBase *GfxBase = NULL;

InterruptVectorT *InterruptVector = (APTR)0L;
InterruptVectorT *GetVBR();

asm(".even\n"
    "_GetVBR:\n"
    "  movec vbr,d0\n"
    "  rte");

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 34))) {
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 34))) {
      /* Get Vector Base Register */
      if (SysBase->AttnFlags & AFF_68010)
        InterruptVector = (APTR)Supervisor((APTR)GetVBR);

      Forbid();

      {
        struct View *OldView = GfxBase->ActiView;
        UWORD OldDMAcon, OldIntena;

        LoadView(NULL);
        WaitBlit();
        OwnBlitter();

        OldDMAcon = custom->dmaconr;
        OldIntena = custom->intenar;

        /* prohibit dma & interrupts */
        custom->dmacon = 0x7fff;
        custom->intena = 0x7fff;

        Main();

        /* restore AmigaOS state of dma & interrupts */
        custom->dmacon = OldDMAcon | DMAF_SETCLR;
        custom->intena = OldIntena | INTF_SETCLR;

        /* restore old copper list */
        custom->cop1lc = (ULONG)GfxBase->copinit;

        DisownBlitter();
        LoadView(OldView);
      }
      Permit();
    }
    CloseLibrary((struct Library *)DOSBase);
  }
  return 0;
}
