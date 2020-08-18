#include <debug.h>
#include <boot.h>
#include <cpu.h>
#include <custom.h>
#include <cia.h>
#include <exception.h>
#include <memory.h>
#include <floppy.h>
#include <filesys.h>

#include "autoinit.h"
#include "sync.h"

u_char CpuModel = CPU_68000;

extern int main(void);

void Loader(BootDataT *bd) {
  Log("[Loader] VBR at $%08x\n", (u_int)bd->bd_vbr);
  Log("[Loader] CPU model $%02x\n", bd->bd_cpumodel);
  Log("[Loader] Entry point at $%08x\n", (u_int)bd->bd_entry);

  CpuModel = bd->bd_cpumodel;
  ExcVecBase = bd->bd_vbr;

  {
    short i;

    for (i = 0; i < bd->bd_nregions; i++) {
      MemRegionT *mr = &bd->bd_region[i];
      AddMemory((void *)mr->mr_lower, mr->mr_upper - mr->mr_lower, mr->mr_attr);
    }
  }

  SetupExceptionVector();

  /* CIA-A & CIA-B: Stop timers and return to default settings. */
  ciaa->ciacra = 0;
  ciaa->ciacrb = 0;
  ciab->ciacra = 0;
  ciab->ciacrb = 0;

  /* CIA-A & CIA-B: Clear pending interrupts. */
  SampleICR(ciaa, CIAICRF_ALL);
  SampleICR(ciab, CIAICRF_ALL);

  /* CIA-A & CIA-B: Disable all interrupts. */
  WriteICR(ciaa, CIAICRF_ALL);
  WriteICR(ciab, CIAICRF_ALL);

  /* Enable master bit in DMACON and INTENA */
  EnableDMA(DMAF_MASTER);
  EnableINT(INTF_INTEN);

  /* Lower interrupt priority level to nominal. */
  SetSR(SR_S);

  InitFloppy();
  InitFileSys();
  InitTracks();
  CallFuncList(&__INIT_LIST__);

  {
    int retval = main();
    Log("[Loader] main() returned %d.\n", retval);
  }

  CallFuncList(&__EXIT_LIST__);
  KillFileSys();
  KillFloppy();
  
  Log("[Loader] Shutdown complete!\n");
}
