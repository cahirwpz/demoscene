#ifndef __COMMON_H__
#define __COMMON_H__

#include <exec/types.h>

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a) < 0) ? (-a) : (a))

#define offsetof(st, m) \
  ((ULONG)((char *)&((st *)0)->m - (char *)0))

static inline int swap16(int a) {
  asm("swap %0": "=d" (a));
  return a;
}

#endif
