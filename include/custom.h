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

static inline void WaitLine(uint32_t line) {
  while ((custom->vposr_ & 0x1ff00) != ((line << 8) & 0x1ff00));
}

static inline void WaitVBlank(void) { WaitLine(303); }

static inline short IsAGA(void) {
  return ((custom->vposr_ >> 24) & 0x7f) == 0x23;
}

#endif /* !__CUSTOM_H__ */
