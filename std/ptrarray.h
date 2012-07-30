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

/*
 * @brief Array of pointers constructor.
 */
PtrArrayT *NewPtrArray(size_t reserved, bool managed);

/*
 * @brief Sets a comparison function useful for sorting.
 */
static inline void PtrArraySetCompareFunc(PtrArrayT *self, CompareFuncT func) {
  self->compareFunc = func;
}

/*
 * @brief Getter & setter functions.
 */

/*
 * @brief Get a pointer to an element without extra checks.
 *
 * Assumes that index is non-negative.  Can return an invalid pointer.
 */
static inline PtrT PtrArrayGet(PtrArrayT *self, size_t index) {
  return self->data[index];
}

static inline PtrT PtrArraySet(PtrArrayT *self, size_t index, PtrT data) {
  PtrT *item = self->data[index];
  *item = data;
  return item;
}

/*
 * @brief Iteration functions.
 */
void PtrArrayForEach(PtrArrayT *self, IterFuncT func, PtrT data);
void PtrArrayForEachInRange(PtrArrayT *self, ssize_t begin, ssize_t end,
                            IterFuncT func, PtrT data);

/*
 * @brief Element adding functions.
 */
PtrT *PtrArrayInsertFast(PtrArrayT *self, ssize_t index, PtrT data);
PtrT *PtrArrayInsert(PtrArrayT *self, ssize_t index, PtrT data);
PtrT *PtrArrayInsertElements(PtrArrayT *self, ssize_t index,
                             PtrT *data, size_t count);
PtrT *PtrArrayAppend(PtrArrayT *self, PtrT data);
PtrT *PtrArrayAppendElements(PtrArrayT *self, PtrT *data, size_t count);

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
                         size_t left, size_t right, PtrT pivot);
void PtrArrayQuickSort(PtrArrayT *self, ssize_t begin, ssize_t end);

#endif
