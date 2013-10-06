#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)
#define INTF_LEVEL4 (INTF_AUD0 | INTF_AUD1 | INTF_AUD2 | INTF_AUD3)

#define offsetof(st, m) \
  ((ULONG)((char *)&((st *)0)->m - (char *)0))

#define BPLCON0_BPU(d)  (((d) & 7) << 12)
#define BPLCON0_COLOR   (1 << 9)

extern volatile struct Custom* const custom;
extern volatile struct CIA* const ciaa;
extern volatile struct CIA* const ciab;

static inline unsigned int swap16(unsigned int a) {
  asm("swap %0": "=d" (a));
  return a;
}

void WaitMouse();
void WaitBlitter();
void WaitVBlank();

#endif
