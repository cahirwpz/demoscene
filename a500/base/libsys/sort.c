#include "sort.h"

static __regargs void InsertSort(SortItemT *first, SortItemT *last) {
  SortItemT *ptr = first + 1;

  while (ptr <= last) {
    SortItemT *curr = ptr;
    SortItemT *prev = ptr - 1;
    SortItemT this = *ptr++;
    while (prev >= first && prev->key > this.key)
      *curr-- = *prev--;
    *curr = this;
  }
}

#define THRESHOLD (int)(12 * sizeof(SortItemT))

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

    if ((void *)right - (void *)first > THRESHOLD)
      QuickSort(first, right - 1);
    else
      InsertSort(first, right - 1);

    if ((void *)last - (void *)left > THRESHOLD)
      QuickSort(left, last);
    else
      InsertSort(left, last);
  }
}

__regargs void SortItemArray(SortItemT *table, short size) {
  QuickSort(table, &table[size - 1]);
}
