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

/* Condition Code Register (lower bits of SR) */
#define CCR_X __BIT(4) /* eXtend */
#define CCR_N __BIT(3) /* Negative */
#define CCR_Z __BIT(2) /* Zero */
#define CCR_V __BIT(1) /* oVerflow */
#define CCR_C __BIT(0) /* Carry */

/* Status Register for 68000 */
#define SR_IM 0x0700   /* Interrupt Mask */
#define SR_S __BIT(13) /* Supervisor Mode */
#define SR_T __BIT(15) /* Trace Mode */

extern u_char CpuModel;

/* Read Vector Base Register (68010 and above only) */
static inline void *GetVBR(void) {
  void *vbr;
  asm volatile("\tmovec\t%%vbr,%0\n" : "=d"(vbr));
  return vbr;
}

/* Read whole Status Register (privileged instruction on 68010 and above) */
static inline u_short GetSR(void) {
  u_short sr;
  asm volatile("\tmove.w\t%%sr,%0\n" : "=d"(sr));
  return sr;
}

/* Read whole Status Register (privileged instruction on 68010 and above) */
static inline void SetSR(u_short sr) {
  asm volatile("\tmove.w\t%0,%%sr\n" :: "di"(sr));
}

/* Make the processor wait for interrupt. */
static inline void CpuWait(void) {
  asm volatile("\tstop\t#0x2000\n");
}

/* Code running in task context executes with IPL set to 0. */
static inline void CpuIntrDisable(void) {
  asm volatile("\tor.w\t#0x0700,%sr\n");
}

static inline void CpuIntrEnable(void) {
  asm volatile("\tand.w\t#0xf8ff,%sr\n");
}

/* Returns if caller is running with all interrupts disabled. */
static inline int CpuIntrDisabled(void) {
  return (GetSR() & 0x0700) == 0x0700;
}

/* Code running in interrupt context may be interrupted on M68000 by higher
 * priority level interrupt. To construct critical section we need to use IPL
 * bits in SR register. Returns previous value of IPL. */
u_short SetIPL(u_short);

#endif
