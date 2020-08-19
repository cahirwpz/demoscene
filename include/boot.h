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
  struct Hunk *bd_hunk;
  void *bd_vbr;
  u_short bd_cpumodel;
  u_short bd_nregions;
  MemRegionT bd_region[0];
} BootDataT;

#endif
