#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/graphics.h>

#include "hardware.h"
#include "interrupts.h"
#include "io.h"
#include "startup.h"
#include "tasks.h"

extern EffectT Effect;
extern BOOL execOnly;

LONG frameCount;
LONG lastFrameCount;

static WORD kickVer;
static struct List PortsIntChain;
static struct List CoperIntChain;
static struct List VertbIntChain;
static struct List OrigTaskReady;
static struct List OrigTaskWait;

static struct {
  struct View *view;
  UWORD dmacon, intena, adkcon;
  ULONG cacheBits;
} old;

static void DummyRender() {}
static BOOL ExitOnLMB() { return !LeftMouseButton(); }

#define IDLETASK 0

#if IDLETASK
static void IdleTask() {
  for (;;) {
    custom->color[0] = 0x00f;
  }
}
#endif

void KillOS() {
  Log("[Startup] Save AmigaOS state.\n");

  /* Allocate blitter. */
  WaitBlit();
  OwnBlitter();

  /* No calls to any other library than exec beyond this point or expect
   * undefined behaviour including crashes. */
  execOnly = TRUE;
  Forbid();

  /* Disable CPU caches. */
  if (kickVer >= 36)
    old.cacheBits = CacheControl(0, -1);

  /* Intercept the view of AmigaOS. */
  old.view = GfxBase->ActiView;
  LoadView(NULL);
  WaitTOF();
  WaitTOF();

  /* DMA & interrupts take-over. */
  old.adkcon = custom->adkconr;
  old.dmacon = custom->dmaconr;
  old.intena = custom->intenar;

  /* Prohibit dma & interrupts. */
  custom->adkcon = (UWORD)~ADKF_SETCLR;
  custom->dmacon = (UWORD)~DMAF_SETCLR;
  custom->intena = (UWORD)~INTF_SETCLR;
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  /* Enable master switches...
   * .. and SOFTINT which is presumably used by Exec's scheduler. */
  custom->dmacon = DMAF_SETCLR | DMAF_MASTER;
  custom->intena = INTF_SETCLR | INTF_INTEN | INTF_SOFTINT;

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

  /* Save original task lists. */
  CopyMem(&SysBase->TaskReady, &OrigTaskReady, sizeof(struct List));
  CopyMem(&SysBase->TaskWait, &OrigTaskWait, sizeof(struct List));

  /* Reset system's task lists. */
  NewList(&SysBase->TaskReady);
  NewList(&SysBase->TaskWait);

  /* Restore multitasking. */
  Permit();

  SetTaskPri(SysBase->ThisTask, 0);
}

void RestoreOS() {
  Log("[Startup] Restore AmigaOS state.\n");

  /* Suspend multitasking. */
  Forbid();

  /* firstly... disable dma and interrupts that were used in Main */
  custom->dmacon = (UWORD)~DMAF_SETCLR;
  custom->intena = (UWORD)~INTF_SETCLR;
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (UWORD)~INTF_SETCLR;
  custom->intreq = (UWORD)~INTF_SETCLR;

  /* Restore original task lists. */
  CopyMem(&OrigTaskReady, &SysBase->TaskReady, sizeof(struct List));
  CopyMem(&OrigTaskWait, &SysBase->TaskWait, sizeof(struct List));

  /* Restore original interrupt server chains. */
  CopyMem(&PortsIntChain, SysBase->IntVects[INTB_PORTS].iv_Data,
          sizeof(struct List));
  CopyMem(&CoperIntChain, SysBase->IntVects[INTB_COPER].iv_Data,
          sizeof(struct List));
  CopyMem(&VertbIntChain, SysBase->IntVects[INTB_VERTB].iv_Data, 
          sizeof(struct List));

  /* Restore AmigaOS state of dma & interrupts. */
  custom->dmacon = old.dmacon | DMAF_SETCLR;
  custom->intena = old.intena | INTF_SETCLR;
  custom->adkcon = old.adkcon | ADKF_SETCLR;

  /* Restore old copper list... */
  custom->cop1lc = (ULONG)GfxBase->copinit;
  WaitVBlank();

  /* ... and original view. */
  LoadView(old.view);
  WaitTOF();
  WaitTOF();

  /* Enable CPU caches. */
  if (kickVer >= 36)
    CacheControl(old.cacheBits, -1);

  /* Restore multitasking. */
  Permit();
  execOnly = FALSE;

  /* Deallocate blitter. */
  DisownBlitter();
}

/* VBlank event list. */
struct List *VBlankEvent = &(struct List){};

/* Wake up tasks asleep in wait for VBlank interrupt. */
static LONG VBlankEventHandler() {
  TaskSignalIntr(VBlankEvent);
  return 0;
}

INTERRUPT(VBlankWakeUp, 10, VBlankEventHandler, NULL);

int main() {
#if IDLETASK
  struct Task *idleTask = NULL;
#endif

  if (Effect.Load) {
    Effect.Load();
    Log("[Main] Effect loading finished\n");
  }

  KillOS();

#if IDLETASK
  idleTask = CreateTask("IdleTask", -10, IdleTask, 1024);
#endif

  NewList(VBlankEvent);
  AddIntServer(INTB_VERTB, VBlankWakeUp);

  if (!Effect.Render)
    Effect.Render = DummyRender;
  if (!Effect.HandleEvent)
    Effect.HandleEvent = ExitOnLMB;

  if (Effect.Init) {
    Effect.Init();
    Log("[Main] Effect initialization done\n");
  }

  lastFrameCount = ReadFrameCounter();

  while (Effect.HandleEvent()) {
    LONG t = ReadFrameCounter();
    frameCount = t;
    Effect.Render();
    lastFrameCount = t;
  }

  if (Effect.Kill)
    Effect.Kill();

  RemIntServer(INTB_VERTB, VBlankWakeUp);

#if IDLETASK
  RemTask(idleTask);
#endif

  RestoreOS();

  if (Effect.UnLoad)
    Effect.UnLoad();

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

  Log("[Main] ROM: %ld.%ld, CPU: 680%ld0, CHIP: %ldkB, FAST: %ldkB\n",
      (LONG)kickVer, (LONG)kickRev, (LONG)cpu,
      (LONG)(AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024),
      (LONG)(AvailMem(MEMF_FAST | MEMF_LARGEST) / 1024));
}

typedef struct LibDesc {
  struct Library *base;
  char *name;
} LibDescT;

extern LibDescT *__LIB_LIST__[];

#define OSLIBVERSION 33

void InitLibraries() {
  LibDescT **list = __LIB_LIST__;
  ULONG numbases = (ULONG)*list++;

  while (numbases-- > 0) {
    LibDescT *lib = *list++;
    if (!(lib->base = OpenLibrary(lib->name, OSLIBVERSION))) {
      Log("Cannot open '%s'!\n", lib->name);
      exit(20);
    }
  }
}

void KillLibraries() {
  LibDescT **list = __LIB_LIST__;
  ULONG numbases = (ULONG)*list++;

  while (numbases-- > 0) {
    LibDescT *lib = *list++;
    if (lib->base)
      CloseLibrary(lib->base);
  }
}

ADD2INIT(InitLibraries, -120);
ADD2EXIT(KillLibraries, -120);

ADD2INIT(SystemInfo, -50);
