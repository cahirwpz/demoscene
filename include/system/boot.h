#ifndef __BOOT_H__
#define __BOOT_H__

#include <types.h>

struct Hunk;

typedef struct __packed MemRegion {
  uintptr_t mr_lower;
  uintptr_t mr_upper;
  u_short mr_attr;
} MemRegionT;

typedef struct BootData {
  struct Hunk *bd_hunk; /* First hunk of executable file */
  void *bd_vbr;         /* Vector Base Register (for 68010+) */
  void *bd_stkbot;      /* Stack bottom pointer */
  u_int bd_stksz;       /* Stack size */
  u_short bd_cpumodel;  /* Processor model */
  u_short bd_nregions;  /* Number of memory regions */
  MemRegionT bd_region[0];
} BootDataT;

#endif
