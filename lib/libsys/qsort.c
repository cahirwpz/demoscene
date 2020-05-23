#include "qsort.h"

static inline void swap(void *x, void *y, short l) {
  u_char *a = x, *b = y, c;
  l--;
  do {
    c = *a;
    *a++ = *b;
    *b++ = c;
  } while (--l != -1);
}

static __regargs void sort(void *first, void *last, int size, 
                           __regargs int (*cmp)(const void *, const void *))
{
  if (last - first > size) {
    void *pivot = first;
    void *left = first + size;
    void *right = last;

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

void qsort(void *array, int nitems, int size,
           __regargs int (*cmp)(const void *, const void *))
{
  sort(array, array + (nitems - 1) * size, size, cmp);
}
