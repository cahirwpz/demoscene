#ifndef __SYSTEM_CPU_H__
#define __SYSTEM_CPU_H__

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

/* Interrupt priority level. If current IPL is greater than 0 then processor is
 * running in interrupt context. Otherwise is running in task context. */
#define IPL(x) (((x) & 7) << 8)
#define IPL_MAX IPL(7)
#define IPL_NONE IPL(0)

extern u_char CpuModel;

/* Make the processor wait for interrupt. */
static inline void CpuWait(void) {
  asm volatile("stop #0x2000");
}

/* Code running in task context executes with IPL set to 0. */
static inline void CpuIntrDisable(void) {
  asm volatile("or.w #0x0700,%sr");
}

static inline void CpuIntrEnable(void) {
  asm volatile("and.w #0xf8ff,%sr");
}

/* Returns current interrupt priority level. Reads whole Status Register
 * (it's privileged instruction on 68010 and above). */
static inline u_short GetIPL(void) {
  u_short sr;
  asm volatile("move.w %%sr,%0" : "=d"(sr));
  return sr & SR_IM;
}

/* Code running in interrupt context may be interrupted on M68000 by higher
 * priority level interrupt. To construct critical section we need to use IPL
 * bits in SR register. Returns previous value of IPL. */
u_short SetIPL(u_short);

#endif /* !__SYSTEM_CPU_H__ */
