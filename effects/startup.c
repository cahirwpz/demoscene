#include <exec/execbase.h>
#include <proto/alib.h>
#include <proto/exec.h>

#include "hardware.h"
#include "interrupts.h"
#include "io.h"
#include "startup.h"
#include "tasks.h"

extern EffectT Effect;

int frameCount;
int lastFrameCount;

static short kickVer;
static struct List PortsIntChain;
static struct List CoperIntChain;
static struct List VertbIntChain;
static struct List ExterIntChain;
static struct List OrigTaskReady;
static struct List OrigTaskWait;

static struct {
  u_short dmacon, intena, adkcon;
  u_int cacheBits;
} old;

static void DummyRender(void) {}
static bool ExitOnLMB(void) { return !LeftMouseButton(); }

#define IDLETASK 0

#if IDLETASK
static void IdleTask(void) {
  for (;;) {
    custom->color[0] = 0x00f;
  }
}
#endif

void KillOS(void) {
  Log("[Startup] Save AmigaOS state.\n");

  /* No calls to any other library than exec beyond this point or expect
   * undefined behaviour including crashes. */
  Forbid();

  /* Disable CPU caches. */
  if (kickVer >= 36)
    old.cacheBits = CacheControl(0, -1);

  /* DMA & interrupts take-over. */
  old.adkcon = custom->adkconr;
  old.dmacon = custom->dmaconr;
  old.intena = custom->intenar;

  /* Prohibit dma & interrupts. */
  custom->adkcon = (u_short)~ADKF_SETCLR;
  custom->dmacon = (u_short)~DMAF_SETCLR;
  custom->intena = (u_short)~INTF_SETCLR;
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (u_short)~INTF_SETCLR;
  custom->intreq = (u_short)~INTF_SETCLR;

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
  CopyMem(SysBase->IntVects[INTB_EXTER].iv_Data, 
          &ExterIntChain, sizeof(struct List));

  /* Reset system's interrupt server chains. */
  NewList(SysBase->IntVects[INTB_PORTS].iv_Data);
  NewList(SysBase->IntVects[INTB_COPER].iv_Data);
  NewList(SysBase->IntVects[INTB_VERTB].iv_Data);
  NewList(SysBase->IntVects[INTB_EXTER].iv_Data);

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

ADD2INIT(KillOS, -20);

void RestoreOS(void) {
  Log("[Startup] Restore AmigaOS state.\n");

  /* Suspend multitasking. */
  Forbid();

  /* firstly... disable dma and interrupts that were used in Main */
  custom->dmacon = (u_short)~DMAF_SETCLR;
  custom->intena = (u_short)~INTF_SETCLR;
  WaitVBlank();

  /* Clear all interrupt requests. Really. */
  custom->intreq = (u_short)~INTF_SETCLR;
  custom->intreq = (u_short)~INTF_SETCLR;

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
  CopyMem(&ExterIntChain, SysBase->IntVects[INTB_EXTER].iv_Data, 
          sizeof(struct List));

  /* Restore AmigaOS state of dma & interrupts. */
  custom->dmacon = old.dmacon | DMAF_SETCLR;
  custom->intena = old.intena | INTF_SETCLR;
  custom->adkcon = old.adkcon | ADKF_SETCLR;

  /* Enable CPU caches. */
  if (kickVer >= 36)
    CacheControl(old.cacheBits, -1);

  /* Restore multitasking. */
  Permit();
}

ADD2EXIT(RestoreOS, -20);

/* VBlank event list. */
struct List *VBlankEvent = &(struct List){NULL, NULL, NULL, 0, 0};

/* Wake up tasks asleep in wait for VBlank interrupt. */
static int VBlankEventHandler(void) {
  TaskSignalIntr(VBlankEvent);
  return 0;
}

INTERRUPT(VBlankWakeUp, 10, VBlankEventHandler, NULL);

int main(void) {
#if IDLETASK
  struct Task *idleTask = CreateTask("IdleTask", -10, IdleTask, 1024);
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

  SetFrameCounter(0);

  lastFrameCount = ReadFrameCounter();

  while (Effect.HandleEvent()) {
    int t = ReadFrameCounter();
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

  return 0;
}

void SystemInfo(void) {
  short kickRev;
  short cpu = 0;

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
    void *kickEnd = (void *)0x1000000;
    u_int kickSize = *(u_int *)(kickEnd - 0x14);
    u_short *kick = kickEnd - kickSize;

    kickVer = kick[6];
    kickRev = kick[7];
  }

  Log("[Main] ROM: %d.%d, CPU: 680%d0, CHIP: %ldkB, FAST: %ldkB\n",
      kickVer, kickRev, cpu,
      AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024,
      AvailMem(MEMF_FAST | MEMF_LARGEST) / 1024);
}

ADD2INIT(SystemInfo, -50);

void LoadEffect(void) {
  if (Effect.Load) {
    Effect.Load();
    Log("[Main] Effect loading finished\n");
  }
}

ADD2INIT(LoadEffect, 0);

void UnLoadEffect(void) {
  if (Effect.UnLoad)
    Effect.UnLoad();
}

ADD2EXIT(UnLoadEffect, 0);
