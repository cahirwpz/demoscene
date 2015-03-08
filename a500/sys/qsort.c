#include "qsort.h"

static inline void swap(APTR x, APTR y, WORD l) {
  UBYTE *a = x, *b = y, c;
  l--;
  do {
    c = *a;
    *a++ = *b;
    *b++ = c;
  } while (--l != -1);
}

static __regargs void sort(APTR first, APTR last, LONG size, 
                           __regargs LONG (*cmp)(CONST APTR, CONST APTR))
{
  if (last - first > size) {
    CONST APTR pivot = first;
    APTR left = first + size;
    APTR right = last;

    while (left < right) {
      while ((cmp(left, pivot) <= 0) && (left < last))
        left += size;
      while ((cmp(right, pivot) > 0) && (right > first))
        right -= size;
      if (left < right)
        swap(left, right, size);
    }

    swap(pivot, right, size);
    sort(first, right - size, size, cmp);
    sort(left, last, size, cmp);
  }
}

void qsort(APTR array, LONG nitems, LONG size, __regargs LONG (*cmp)(CONST APTR, CONST APTR)) {
  sort(array, array + (nitems - 1) * size, size, cmp);
}
