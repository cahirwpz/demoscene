#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)

#define offsetof(st, m) \
  ((ULONG)((char *)&((st *)0)->m - (char *)0))

extern volatile struct Custom *custom;
extern volatile struct CIA *ciaa;
extern volatile struct CIA *ciab;

static inline unsigned int swap16(unsigned int a) {
  asm("swap %0": "=d" (a));
  return a;
}

void WaitMouse();
void WaitBlitter();
void WaitVBlank();

#endif
