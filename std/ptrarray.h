#ifndef __STD_PTRARRAY_H__
#define __STD_PTRARRAY_H__

#include "std/types.h"

typedef struct PtrArray {
  PtrT *data;

  bool   managed;
  size_t size;
  size_t reserved;

  FreeFuncT freeFunc;
  CompareFuncT compareFunc;
} PtrArrayT;

PtrArrayT *NewPtrArray(size_t reserved, bool managed);

/*
 * @brief Sets a comparison function useful for sorting.
 */
static inline void PtrArraySetCompareFunc(PtrArrayT *self, CompareFuncT func) {
  self->compareFunc = func;
}

/*
 * @brief Get a pointer to an element without extra checks.
 *
 * Assumes that index is non-negative.  Can return an invalid pointer.
 */
static inline PtrT PtrArrayGetFast(PtrArrayT *self, size_t index) {
  return self->data[index];
}

/*
 * @brief Swap two elements without extra checks.
 *
 * Assumes that fst != snd.
 */
static inline void PtrArraySwapFast(PtrArrayT *self, size_t i, size_t j) {
  PtrT *data = self->data;
  PtrT tmp;
 
  tmp = data[i];
  data[i] = data[j];
  data[j] = tmp;
}

/*
 * @brief Getter & setter functions.
 */
PtrT PtrArrayGet(PtrArrayT *self asm("a0"), ssize_t index asm("d0"));
void PtrArraySet(PtrArrayT *self asm("a0"), ssize_t index asm("d0"),
                 PtrT data asm("a1"));

/*
 * @brief Iteration functions.
 */
void PtrArrayForEach(PtrArrayT *self, IterFuncT func, PtrT data);
void PtrArrayForEachInRange(PtrArrayT *self, ssize_t begin, ssize_t end,
                            IterFuncT func, PtrT data);

/*
 * @brief Element adding functions.
 */
void PtrArrayInsertFast(PtrArrayT *self, ssize_t index, PtrT data);
void PtrArrayInsert(PtrArrayT *self, ssize_t index, PtrT data);
void PtrArrayInsertElements(PtrArrayT *self, ssize_t index,
                            PtrT *data, size_t count);
void PtrArrayAppend(PtrArrayT *self, PtrT data);
void PtrArrayAppendElements(PtrArrayT *self, PtrT *data, size_t count);

/*
 * @brief Remove a pointer and fill in the gap with last pointer in the array.
 *
 * Assumes that index is non-negative.
 */
void PtrArrayRemoveFast(PtrArrayT *self, size_t index);
void PtrArrayRemove(PtrArrayT *self, ssize_t index);
void PtrArrayRemoveRange(PtrArrayT *self, ssize_t first, ssize_t last);
void PtrArrayFilterFast(PtrArrayT *self, PredicateT func);

/*
 * Array size management functions.
 */
void PtrArrayResize(PtrArrayT *self, size_t newSize);

/*
 * Sorting functions.
 */
void PtrArrayInsertionSort(PtrArrayT *self, ssize_t begin, ssize_t end);
size_t PtrArrayPartition(PtrArrayT *self,
                         size_t begin, size_t end, PtrT pivot);
void PtrArrayQuickSort(PtrArrayT *self, ssize_t begin, ssize_t end);

#endif
