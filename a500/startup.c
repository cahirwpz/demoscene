#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/graphics.h>

#include "hardware.h"
#include "interrupts.h"
#include "io.h"
#include "startup.h"

extern void CallHRTmon();
extern EffectT Effect;

int __nocommandline = 1;
ULONG __oslibversion = 33;

LONG frameCount;
LONG lastFrameCount;

static WORD kickVer;
static struct List PortsIntChain;
static struct List CoperIntChain;
static struct List VertbIntChain;

static void DummyRender() {}
static BOOL ExitOnLMB() { return !LeftMouseButton(); }

int main() {
  if (Effect.Load)
    Effect.Load();

  {
    struct View *OldView;
    UWORD OldDmacon, OldIntena, OldAdkcon;
    ULONG OldCacheBits = 0;

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

    /* Enable master switches. */
    custom->dmacon = DMAF_SETCLR | DMAF_MASTER;
    custom->intena = INTF_SETCLR | INTF_INTEN;

    /* Save original interrupt server chains. */
    CopyMem(SysBase->IntVects[INTB_PORTS].iv_Data,
            &PortsIntChain, sizeof(struct List));
    CopyMem(SysBase->IntVects[INTB_COPER].iv_Data,
            &CoperIntChain, sizeof(struct List));
    CopyMem(SysBase->IntVects[INTB_VERTB].iv_Data, 
            &VertbIntChain, sizeof(struct List));

    /* Reset system's interrupt server chains. */
    NewList(SysBase->IntVects[INTB_PORTS].iv_Data);
    NewList(SysBase->IntVects[INTB_COPER].iv_Data);
    NewList(SysBase->IntVects[INTB_VERTB].iv_Data);

    if (!Effect.Render)
      Effect.Render = DummyRender;
    if (!Effect.HandleEvent)
      Effect.HandleEvent = ExitOnLMB;

    if (Effect.Init)
      Effect.Init();

    lastFrameCount = ReadFrameCounter();

    while (Effect.HandleEvent()) {
      LONG t = ReadFrameCounter();
      frameCount = t;
      Effect.Render();
      lastFrameCount = t;
    }

    if (Effect.Kill)
      Effect.Kill();

    /* firstly... disable dma and interrupts that were used in Main */
    custom->dmacon = (UWORD)~DMAF_SETCLR;
    custom->intena = (UWORD)~INTF_SETCLR;
    WaitVBlank();

    /* Clear all interrupt requests. Really. */
    custom->intreq = (UWORD)~INTF_SETCLR;
    custom->intreq = (UWORD)~INTF_SETCLR;

    /* Restore original interrupt server chains. */
    CopyMem(&PortsIntChain, SysBase->IntVects[INTB_PORTS].iv_Data,
            sizeof(struct List));
    CopyMem(&CoperIntChain, SysBase->IntVects[INTB_COPER].iv_Data,
            sizeof(struct List));
    CopyMem(&VertbIntChain, SysBase->IntVects[INTB_VERTB].iv_Data, 
            sizeof(struct List));

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
  }

  return 0;
}

void SystemInfo() {
  WORD kickRev;
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
}

void InitTrapHandler() {
  if (*(LONG *)0xA10004 == MAKE_ID('H', 'R', 'T', '!')) {
    struct Task *tc = FindTask(NULL);
    tc->tc_TrapCode = &CallHRTmon;
    Log("[Init] Installed trap handler.\n");
  }
}

void KillTrapHandler() {
  struct Task *tc = FindTask(NULL);
  tc->tc_TrapCode = NULL;
}

ADD2INIT(InitTrapHandler, -100);
ADD2EXIT(KillTrapHandler, -100);
ADD2INIT(SystemInfo, -50);
