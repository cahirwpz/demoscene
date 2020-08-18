#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <cdefs.h>
#include <types.h>

void qsort(void *array, u_int nitems, u_int size,
           int (*cmp)(const void *, const void *));

u_int random(void);

#endif
