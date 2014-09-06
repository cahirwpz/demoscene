#include "qsort.h"

static inline void swap(APTR x, APTR y, WORD l) {
  UBYTE *a = x, *b = y, c;
  while (l--) {
    c = *a;
    *a++ = *b;
    *b++ = c;
  }
}

__regargs static void sort(APTR array, LONG size,
                           __regargs LONG (*cmp)(APTR, APTR), LONG begin, LONG end) 
{
  if (end > begin) {
    APTR pivot = array + begin;
    LONG l = begin + size;
    LONG r = end;

    while (l < r) {
      if (cmp(array + l, pivot) <= 0) {
        l += size;
      } else if (cmp(array + r, pivot) > 0)  {
        r -= size;
      } else if (l < r) {
        swap(array + l, array + r, size);
      }
    }
    l -= size;

    swap(array + begin, array + l, size);
    sort(array, size, cmp, begin, l);
    sort(array, size, cmp, r, end);
  }
}

void qsort(APTR array, LONG nitems, LONG size, __regargs LONG (*cmp)(APTR, APTR)) {
  sort(array, size, cmp, 0, (nitems - 1) * size);
}
