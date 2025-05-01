#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <cdefs.h>
#include <types.h>

typedef struct {
  long quot;
  long rem;
} ldiv_t;

__stdargs ldiv_t ldivu(u_long numer, u_long denom);
__stdargs ldiv_t ldivs(long numer, long denom);

void qsort(void *array, u_int nitems, u_int size,
           int (*cmp)(const void *, const void *));

u_int random(void);

#endif
