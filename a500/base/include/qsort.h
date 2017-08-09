#ifndef __QSORT_H__
#define __QSORT_H__

#include <exec/types.h>

void qsort(APTR array, LONG nitems, LONG size, __regargs LONG (*cmp)(APTR, APTR));

#endif
