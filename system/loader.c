#include <debug.h>
#include <system/amigahunk.h>
#include <system/autoinit.h>
#include <system/boot.h>
#include <system/cia.h>
#include <system/cpu.h>
#include <system/exception.h>
#include <system/interrupt.h>
#include <system/memory.h>
#include <system/task.h>

u_char CpuModel = CPU_68000;
u_char BootDev;

extern int main(void);

void Loader(BootDataT *bd) {
  Log("[Loader] VBR at $%08x\n", (u_int)bd->bd_vbr);
  Log("[Loader] CPU model $%02x\n", bd->bd_cpumodel);
  Log("[Loader] Stack at $%08x (%d bytes)\n",
      (u_int)bd->bd_stkbot, bd->bd_stksz);

  CpuModel = bd->bd_cpumodel;
  BootDev = bd->bd_bootdev;
  ExcVecBase = bd->bd_vbr;

  {
    short i;

    for (i = 0; i < bd->bd_nregions; i++) {
      MemRegionT *mr = &bd->bd_region[i];
      uintptr_t lower = mr->mr_lower ? mr->mr_lower : 1;
      AddMemory((void *)lower, mr->mr_upper - lower, mr->mr_attr);
    }
  }

#ifndef AMIGAOS
  Log("[Loader] Executable file segments:\n");
  {
    HunkT *hunk = bd->bd_hunk;
    do {
      Log("[Loader] * $%08x - $%08lx\n",
          (u_int)hunk->data, (u_int)hunk->data + hunk->size - sizeof(HunkT));
      hunk = hunk->next;
    } while (hunk);
  }

  Log("[Loader] Setup shared hunks.\n");
  SetupSharedHunks(bd->bd_hunk);
#endif

  SetupExceptionVector(bd);
  SetupInterruptVector();

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
  SetIPL(IPL_NONE);

#if MULTITASK
  TaskInit(CurrentTask, "main", bd->bd_stkbot, bd->bd_stksz);
#endif
  CallFuncList(&__INIT_LIST__);

  {
    __unused int retval = main();
    Log("[Loader] main() returned %d.\n", retval);
  }

  CallFuncList(&__EXIT_LIST__);
  
  Log("[Loader] Shutdown complete!\n");
}
