#ifndef __SYSTEM_BOOT_H__
#define __SYSTEM_BOOT_H__

#include <types.h>
#include <cdefs.h>

struct Hunk;

typedef struct __packed MemRegion {
  uintptr_t mr_lower;
  uintptr_t mr_upper;
  u_short mr_attr;
} MemRegionT;

typedef struct BootData {
  struct Hunk *bd_hunk; /* First hunk of executable file */
  void *bd_vbr;         /* Vector Base Register (for 68010+) */
  void *bd_topaz;       /* topaz.font(8) character data */
  void *bd_stkbot;      /* Stack bottom pointer */
  u_int bd_stksz;       /* Stack size */
  u_char bd_bootdev;    /* 0=floppy, 1=ram/amigaos */
  u_char bd_cpumodel;   /* Processor model */
  u_short bd_nregions;  /* Number of memory regions */
  MemRegionT bd_region[__FLEX_ARRAY];
} BootDataT;

extern u_char BootDev;

#endif /* !__SYSTEM_BOOT_H__ */
