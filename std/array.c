#include <strings.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/array.h"

struct _Array {
  ArrayDataT *array;
  size_t elemSize;
  size_t reserved;
  bool zeroed;
  FreeFuncT freeFunc;
};

static void DeleteArray(ArrayT *self) {
  if (self->freeFunc)
    ArrayForEach(self, (IterFuncT)self->freeFunc, NULL);
  MemUnref(self->array);
}

ArrayT *NewArray(size_t reserved, size_t elemSize, bool zeroed) {
  ArrayT *array = NewRecordGC(ArrayT, (FreeFuncT)DeleteArray);

  array->elemSize = elemSize;
  array->zeroed = zeroed;
  array->reserved = reserved;
  array->array = MemNew0(sizeof(ArrayDataT) + reserved * elemSize, NULL);
  array->array->size = 0;

  return array;
}

ArrayT *NewPtrArray(size_t reserved, bool autoFree) {
  ArrayT *array = NewArray(reserved, sizeof(PtrT), TRUE);

  if (autoFree)
    ArraySetFreeFunc(array, (FreeFuncT)MemUnref);
  
  return array;
}

void ArraySetFreeFunc(ArrayT *self, FreeFuncT func) {
  self->freeFunc = func;
}

ArrayDataT *ArrayGetData(ArrayT *self) {
  return self->array;
}

/*
 * @brief Get a pointer to an element without extra checks.
 *
 * Assumes that index is non-negative.  Can return an invalid pointer.
 */
PtrT ArrayGetFast(ArrayT *self, size_t index) {
  return &self->array->data[self->elemSize * index];
}

/*
 * @brief Swap two elements without extra checks.
 *
 * Assumes that:
 * 1) fst != snd
 * 2) there's one unoccupied element at the end of array
 */
void ArraySwapFast(ArrayT *self, PtrT fst, PtrT snd) {
  PtrT tmp = ArrayGetFast(self, self->array->size - 1);

  memcpy(tmp, fst, self->elemSize);
  memcpy(fst, snd, self->elemSize);
  memcpy(snd, tmp, self->elemSize);
}

static void ArrayClearRange(ArrayT *self, size_t first, size_t last) {
  if (self->zeroed)
    bzero(ArrayGetFast(self, first), self->elemSize * (last - first + 1));
}

static void ArrayFreeRange(ArrayT *self, size_t first, size_t last) {
  if (self->freeFunc) {
    PtrT item = ArrayGetFast(self, first);

    while (first++ <= last) {
      self->freeFunc(item);
      item += self->elemSize;
    }
  }
}

static void ArrayMoveRange(ArrayT *self, size_t index, size_t first, size_t last) {
  size_t remainding = last - first + 1;

  memmove(ArrayGetFast(self, index),
          ArrayGetFast(self, first),
          self->elemSize * remainding);
}

/*
 * @brief Change maximum number of element the array can hold.
 *
 * 1) newSize > current size => extra space will be allocated.
 * 2) newSize < current size => some elements will be removed and
 *    reserved space will be shrinked.
 */
static void ArrayDataResize(ArrayT *self, size_t newSize) {
  ArrayDataT *array = MemDupGC(self->array,
                               sizeof(ArrayDataT) + newSize * self->elemSize,
                               NULL);
  MemUnref(self->array);
  self->array = array;
}

void ArrayResize(ArrayT *self, size_t newSize) {
  if (!self->array) {
    self->array = MemNew(sizeof(ArrayDataT) + newSize * self->elemSize, NULL);
    self->array->size = 0;
    ArrayClearRange(self, 0, newSize - 1);
  } else {
    if (newSize > self->reserved) {
      ArrayDataResize(self, newSize);
      ArrayClearRange(self, self->reserved, newSize - 1);
    }
   
    if (newSize < self->reserved) {
      if (newSize < self->array->size) {
        ArrayFreeRange(self, newSize, self->array->size - 1);
        self->array->size = newSize;
      }
      ArrayDataResize(self, newSize);
    }
  }

  self->reserved = newSize;
}

#define MIN_SIZE 16

size_t NearestPow2(size_t num) {
  size_t i = 1;

  while (num > i)
    i += i;

  return max(i, MIN_SIZE);
}

/*
 * @brief Check if array can accomodate extra elements and resize if needed.
 */
static void ArrayMaybeGrow(ArrayT *self, size_t count) {
  self->array->size += count;
}

/*
 * @brief Check if array doesn't have too much of an extra space and shrink.
 */
static void ArrayMaybeShrink(ArrayT *self, size_t count) {
  self->array->size -= count;
}

/*
 * @brief Check if index is valid and normalise it.
 *
 * Accepts negative indices which point to elements starting from the end of
 * array (eg. -1 is the last element).  Terminates program if index is invalid.
 */

static size_t ArrayCheckIndex(ArrayT *self, ssize_t index) {
  ASSERT((index < self->array->size) && (index <= -self->array->size),
         "Index %d out of bound.", index);

  return (index < 0) ? (self->array->size - index) : index;
}

PtrT ArrayGet(ArrayT *self, ssize_t index) {
  return ArrayGetFast(self, ArrayCheckIndex(self, index));
}

void ArraySet(ArrayT *self, ssize_t index, PtrT data) {
  memcpy(ArrayGet(self, index), data, self->elemSize);
}

void ArrayForEach(ArrayT *self, IterFuncT func, PtrT data) {
  PtrT item = ArrayGetFast(self, 0);
  size_t index = 0;

  while (index++ < self->array->size) {
    func(item, data);
    item += self->elemSize;
  }
}

/*
 * Element removal functions.
 */

static void ArrayRemoveInternal(ArrayT *self, size_t first, size_t last) {
  size_t count = last - first + 1;

  ArrayFreeRange(self, first, last);
  ArrayMoveRange(self, first, last + 1, self->array->size - 1);
  ArrayClearRange(self, self->array->size - count, self->array->size - 1);
  ArrayMaybeShrink(self, count);
}

void ArrayRemove(ArrayT *self, ssize_t index) {
  size_t first = ArrayCheckIndex(self, index);

  ArrayRemoveInternal(self, first, first);
}

void ArrayRemoveRange(ArrayT *self, ssize_t first, ssize_t last) {
  first = ArrayCheckIndex(self, first);
  last = ArrayCheckIndex(self, last);

  ASSERT(first <= last, "Range must start with lesser index.");

  ArrayRemoveInternal(self, first, last);
}

/*
 * @brief Remove an element and fill in the gap with last element of array.
 *
 * Assumes that index is non-negative.
 */
void ArrayRemoveFast(ArrayT *self, size_t index) {
  PtrT item = ArrayGetFast(self, index);
  PtrT last = ArrayGetFast(self, self->array->size - 1);

  if (self->freeFunc)
      self->freeFunc(item);

  if (item != last)
    memcpy(item, last, self->elemSize);

  if (self->zeroed)
    bzero(last, self->elemSize);
}

PtrT ArrayInsert(ArrayT *self, ssize_t index) {
  index = ArrayCheckIndex(self, index);

  ArrayMaybeGrow(self, 1);
  ArrayMoveRange(self, index + 1, index, self->array->size - 2);
  return ArrayGetFast(self, index);
}

PtrT ArrayInsertFast(ArrayT *self, ssize_t index) {
  ArrayMaybeGrow(self, 1);

  {
    PtrT item = ArrayGetFast(self, 0);
    PtrT last = ArrayGetFast(self, self->array->size - 1);

    memcpy(last, item, self->elemSize);

    if (self->zeroed)
      bzero(item, self->elemSize);

    return item;
  }
}

void ArrayInsertElements(ArrayT *self, ssize_t index, PtrT data, size_t count) {
  index = ArrayCheckIndex(self, index);

  ArrayMaybeGrow(self, count);
  ArrayMoveRange(self, index + count, index, self->array->size - count - 1);
  memcpy(ArrayGetFast(self, index), data, count * self->elemSize);
}

PtrT ArrayAppend(ArrayT *self) {
  ArrayMaybeGrow(self, 1);

  return ArrayGetFast(self, self->array->size - 1);
}

void ArrayAppendElements(ArrayT *self, PtrT data, size_t count) {
  ArrayMaybeGrow(self, count);

  {
    PtrT last = ArrayGetFast(self, self->array->size - count);
    memcpy(last, data, count * self->elemSize);
  }
}

void ArraySort(ArrayT *self, CompareFuncT func, ssize_t first, ssize_t last) {
}
