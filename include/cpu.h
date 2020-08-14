#ifndef __CPU_H__
#define __CPU_H__

#include <types.h>

typedef enum {
  CPU_68000 = 0,
  CPU_68010 = 1,
  CPU_68020 = 2,
  CPU_68030 = 3,
  CPU_68040 = 4,
  CPU_68060 = 6
} CpuModelT;

typedef enum {
  FPU_NONE = 0,
  FPU_68881 = 1,
  FPU_68882 = 2,
  FPU_68040 = 3
} FpuModelT;

extern u_char CpuModel;
extern u_char FpuModel;

/* Read Vector Base Register (68010 and above only) */
static inline void *portGetVBR(void) {
  void *vbr;
  asm volatile("\tmovec\t%%vbr,%0\n" : "=d"(vbr));
  return vbr;
}

/* Read whole Status Register (privileged instruction on 68010 and above) */
static inline u_short portGetSR(void) {
  u_short sr;
  asm volatile("\tmove.w\t%%sr,%0\n" : "=d"(sr));
  return sr;
}

#endif
