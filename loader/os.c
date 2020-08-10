#include <exec/execbase.h>
#include <proto/alib.h>
#include <proto/exec.h>

#include "debug.h"
#include "hardware.h"
#include "interrupts.h"

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

