#include "sort.h"

/* Insertion sort algorithm. */
__regargs void SortItemArray(SortItemT *table, WORD size) {
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
