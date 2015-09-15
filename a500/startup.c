#include <exec/types.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include "hardware.h"
#include "interrupts.h"
#include "keyboard.h"
#include "mouse.h"
#include "io.h"
#include "startup.h"

extern EffectT Effect;

int __nocommandline = 1;
ULONG __oslibversion = 33;

LONG frameCount;
LONG lastFrameCount;

static __interrupt_handler void IntLevel2Handler() {
  /* Make sure all scratchpad registers are saved, because we call a function
   * that relies on the fact that it's caller responsibility to save them. */
  asm volatile("" ::: "d0", "d1", "a0", "a1");

  if (keyboardActive && (custom->intreqr & INTF_PORTS))
    KeyboardIntHandler();

  custom->intreq = INTF_PORTS;
  custom->intreq = INTF_PORTS;
}

static __interrupt_handler void IntLevel3Handler() {
  asm volatile("" ::: "d0", "d1", "a0", "a1");

  if (mouseActive && (custom->intreqr & INTF_VERTB))
    MouseIntHandler();

  if (Effect.InterruptHandler)
    Effect.InterruptHandler();

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

static void DummyRender() {}
static BOOL ExitOnLMB() { return !LeftMouseButton(); }

int main() {
  WORD kickVer, kickRev;
  WORD cpu = 0;

  if (SysBase->AttnFlags & AFF_68060)
    cpu = 6;
  else if (SysBase->AttnFlags & AFF_68040)
    cpu = 4;
  else if (SysBase->AttnFlags & AFF_68030)
    cpu = 3;
  else if (SysBase->AttnFlags & AFF_68020)
    cpu = 2;
  else if (SysBase->AttnFlags & AFF_68010)
    cpu = 1;

  /* Get Vector Base Register */
  if (cpu > 0)
    InterruptVector = (APTR)Supervisor((APTR)GetVBR);

  /* Based on WhichAmiga method. */
  {
    APTR kickEnd = (APTR)0x1000000;
    ULONG kickSize = *(ULONG *)(kickEnd - 0x14);
    UWORD *kick = kickEnd - kickSize;

    kickVer = kick[6];
    kickRev = kick[7];
  }

  Print("ROM: %ld.%ld, CPU: 680%ld0, CHIP: %ldkB, FAST: %ldkB.\n",
        (LONG)kickVer, (LONG)kickRev, (LONG)cpu,
        (LONG)(AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024),
        (LONG)(AvailMem(MEMF_FAST | MEMF_LARGEST) / 1024));

  if (Effect.Load)
    Effect.Load();

  {
    struct View *OldView;
    UWORD OldDmacon, OldIntena, OldAdkcon;
    ULONG OldCacheBits = 0;

    {
      struct Task *tc = FindTask(NULL);
      tc->tc_TrapCode = TrapHandler;
    }

    /* Allocate blitter. */
    WaitBlit();
    OwnBlitter();

    /* No calls to any other library than exec beyond this point or expect
     * undefined behaviour including crashes. */
    Forbid();

    /* Disable CPU caches. */
    if (kickVer >= 36)
      OldCacheBits = CacheControl(0, -1);

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

    if (!Effect.Render)
      Effect.Render = DummyRender;
    if (!Effect.HandleEvent)
      Effect.HandleEvent = ExitOnLMB;

    InterruptVector->IntLevel2 = IntLevel2Handler;
    InterruptVector->IntLevel3 = IntLevel3Handler;

    if (Effect.Init)
      Effect.Init();

    if (keyboardActive)
      custom->intena = INTF_SETCLR | INTF_PORTS;
    if (mouseActive)
      custom->intena = INTF_SETCLR | INTF_VERTB;

    lastFrameCount = ReadFrameCounter();

    while (Effect.HandleEvent()) {
      LONG t = ReadFrameCounter();
      frameCount = t;
      Effect.Render();
      lastFrameCount = t;
    }

    custom->intena = INTF_VERTB | INTF_PORTS;

    if (Effect.Kill)
      Effect.Kill();

    /* firstly... disable dma and interrupts that were used in Main */
    custom->dmacon = (UWORD)~DMAF_SETCLR;
    custom->intena = (UWORD)~INTF_SETCLR;
    WaitVBlank();

    /* Clear all interrupt requests. Really. */
    custom->intreq = (UWORD)~INTF_SETCLR;
    custom->intreq = (UWORD)~INTF_SETCLR;

    RestoreInterrupts();

    /* Restore old copper list... */
    custom->cop1lc = (ULONG)GfxBase->copinit;
    WaitVBlank();

    /* Restore AmigaOS state of dma & interrupts. */
    custom->dmacon = OldDmacon | DMAF_SETCLR;
    custom->intena = OldIntena | INTF_SETCLR;
    custom->adkcon = OldAdkcon | ADKF_SETCLR;

    /* ... and original view. */
    LoadView(OldView);
    WaitTOF();
    WaitTOF();

    /* Enable CPU caches. */
    if (kickVer >= 36)
      CacheControl(OldCacheBits, -1);

    /* Restore multitasking. */
    Permit();

    /* Deallocate blitter. */
    DisownBlitter();

    if (Effect.UnLoad)
      Effect.UnLoad();

    {
      struct Task *tc = FindTask(NULL);
      tc->tc_TrapCode = NULL;
    }
  }

  return 0;
}
