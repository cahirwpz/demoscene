#ifndef __CUSTOM_H__
#define __CUSTOM_H__

#include <custom_regdef.h>

typedef volatile struct Custom *const CustomPtrT;

extern struct Custom volatile _custom;

#define custom (&_custom)

/* Macros below take or'ed DMAF_* flags. */
static inline void EnableDMA(uint16_t x) { custom->dmacon = DMAF_SETCLR | x; }
static inline void DisableDMA(uint16_t x) { custom->dmacon = x; }

static inline bool RightMouseButton(void) {
  return !(custom->potinp & DATLY);
}

static inline short IsAGA(void) {
  return ((custom->vposr >> 8) & CHIPID_MASK) == CHIPID_ALICE_V3;
}

u_short SetBplcon3(u_short val, u_short mask);

#endif /* !__CUSTOM_H__ */
