#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>

#include <proto/exec.h>
#include <inline/graphics.h>
#include <inline/dos.h> 

#include "hardware.h"
#include "interrupts.h"
#include "print.h"

extern void Load();
extern void Kill();
extern void Main();

int __nocommandline = 1;
int __initlibraries = 0;

struct DosLibrary *DOSBase = NULL;
struct GfxBase *GfxBase = NULL;

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 34))) {
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 34))) {
      /* Get Vector Base Register */
      if (SysBase->AttnFlags & AFF_68010)
        InterruptVector = (APTR)Supervisor((APTR)GetVBR);

      Print("Running on Kickstart %ld.%ld.\n\r",
            (LONG)SysBase->LibNode.lib_Version,
            (LONG)SysBase->LibNode.lib_Revision);

      Load();

      /* Allocate blitter. */
      WaitBlit();
      OwnBlitter();

      {
        struct View *OldView;
        UWORD OldDmacon, OldIntena, OldAdkcon;

        /* No calls to any other library than exec beyond this point or expect
         * undefined behaviour including crashes. */
        Forbid();

        /* Intercept the view of AmigaOS. */
        OldView = GfxBase->ActiView;
        LoadView(NULL);
        WaitTOF();
        WaitTOF();

        /* DMA & interrupts take-over. */
        OldAdkcon = custom->adkconr;
        OldDmacon = custom->dmaconr;
        OldIntena = custom->intenar;

        /* Prohibit dma & interrupts. */
        custom->dmacon = (UWORD)~DMAF_SETCLR;
        custom->intena = (UWORD)~INTF_SETCLR;
        WaitVBlank();

        /* Clear all interrupt requests. Really. */
        custom->intreq = (UWORD)~INTF_SETCLR;
        custom->intreq = (UWORD)~INTF_SETCLR;

        SaveInterrupts();

        /* Enable master switches. */
        custom->dmacon = DMAF_SETCLR | DMAF_MASTER;
        custom->intena = INTF_SETCLR | INTF_INTEN;

        Main();

        /* firstly... disable dma and interrupts that were used in Main */
        custom->dmacon = (UWORD)~DMAF_SETCLR;
        custom->intena = (UWORD)~INTF_SETCLR;
        WaitVBlank();

        /* Clear all interrupt requests. Really. */
        custom->intreq = (UWORD)~INTF_SETCLR;
        custom->intreq = (UWORD)~INTF_SETCLR;

        RestoreInterrupts();

        /* Restore AmigaOS state of dma & interrupts. */
        custom->dmacon = OldDmacon | DMAF_SETCLR;
        custom->intena = OldIntena | INTF_SETCLR;
        custom->adkcon = OldAdkcon | ADKF_SETCLR;

        /* Restore old copper list... */
        custom->cop1lc = (ULONG)GfxBase->copinit;
        /* ... and original view. */
        LoadView(OldView);
        WaitTOF();
        WaitTOF();

        Permit();
      }

      /* Deallocate blitter. */
      DisownBlitter();

      Kill();

      CloseLibrary((struct Library *)GfxBase);
    }

    CloseLibrary((struct Library *)DOSBase);
  }

  return 0;
}
