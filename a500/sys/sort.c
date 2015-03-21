#include "sort.h"

#if 0
static __regargs void InsertionSort(SortItemT *table, WORD size) {
  if (size >= 2) {
    SortItemT *ptr = table + 1;
    register WORD n asm("d7") = size - 2;

    do {
      SortItemT *curr = ptr;
      SortItemT *prev = ptr - 1;
      SortItemT this = *ptr++;
      while (prev >= table && prev->key > this.key)
        *curr-- = *prev--;
      *curr = this;
    } while (--n != -1);
  }
}
#endif

static __regargs void QuickSort(SortItemT *first, SortItemT *last) {
  if (last > first) {
    SortItemT *pivot = first;
    SortItemT *left = first + 1;
    SortItemT *right = last;

    while (left < right) {
      while ((left->key <= pivot->key) && (left < last))
        left++;
      while ((right->key > pivot->key) && (right > first))
        right--;
      if (left < right) {
        SortItemT tmp = *left; *left = *right; *right = tmp;
      }
    }

    { SortItemT tmp = *pivot; *pivot = *right; *right = tmp; }

    QuickSort(first, right - 1);
    QuickSort(left, last);
  }
}

__regargs void SortItemArray(SortItemT *table, WORD size) {
  QuickSort(table, &table[size - 1]);
}
