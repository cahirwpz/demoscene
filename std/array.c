#include <strings.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/array.h"

static void DeleteArray(ArrayT *self) {
  if (self->freeFunc)
    ArrayForEach(self, (IterFuncT)self->freeFunc, NULL);
  MemUnref(self->data);
}

ArrayT *NewArray(size_t reserved, size_t elemSize, bool zeroed) {
  ArrayT *array = NewRecordGC(ArrayT, (FreeFuncT)DeleteArray);

  array->elemSize = elemSize;
  array->zeroed = zeroed;
  array->reserved = 0;

  ArrayResize(array, reserved);

  return array;
}

ArrayT *NewPtrArray(size_t reserved, bool autoFree) {
  ArrayT *array = NewArray(reserved, sizeof(PtrT), TRUE);

  if (autoFree)
    ArraySetFreeFunc(array, (FreeFuncT)MemUnref);
  
  return array;
}

void ArraySetCompareFunc(ArrayT *self, CompareFuncT func) {
  self->compareFunc = func;
}

void ArraySetFreeFunc(ArrayT *self, FreeFuncT func) {
  self->freeFunc = func;
}

/*
 * @brief Get a pointer to an element without extra checks.
 *
 * Assumes that index is non-negative.  Can return an invalid pointer.
 */
PtrT ArrayGetFast(ArrayT *self asm("a0"), size_t index asm("d0")) {
  return &self->data[self->elemSize * index];
}

/*
 * @brief Swap two elements without extra checks.
 *
 * Assumes that fst != snd.
 */
static void ArraySwapFast(ArrayT *self asm("a0"), PtrT fst asm("a1"), PtrT snd asm("a2")) {
  PtrT tmp = self->temporary;
  size_t elemSize = self->elemSize;

  memcpy(tmp, fst, elemSize);
  memcpy(fst, snd, elemSize);
  memcpy(snd, tmp, elemSize);
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
 *
 * Note: Allocates one more element that will serve the purpose of temporary
 * element for some operations (i.e. sort, swap).
 */
void ArrayResize(ArrayT *self, size_t newSize) {
  if (self->data) {
    if (newSize == self->size)
      return;

    if (newSize < self->size) {
      ArrayFreeRange(self, newSize, self->size - 1);
      self->size = newSize;
    }
  }

  {
    size_t elemSize = self->elemSize;
    size_t bytes = (newSize + 1) * elemSize;

    if (!self->data) {
      self->data = MemNew(bytes, NULL);
      self->size = 0;
    } else {
      PtrT oldData = self->data;
      self->data = MemDupGC(oldData, bytes, NULL);
      MemUnref(oldData);
    }

    if (newSize > self->reserved)
      ArrayClearRange(self, self->reserved, newSize);

    self->reserved = newSize;
    self->temporary = &self->data[newSize * elemSize];
  }
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
  self->size += count;
}

/*
 * @brief Check if array doesn't have too much of an extra space and shrink.
 */
static void ArrayMaybeShrink(ArrayT *self, size_t count) {
  self->size -= count;
}

/*
 * @brief Check if index is valid and normalise it.
 *
 * Accepts negative indices which point to elements starting from the end of
 * array (eg. -1 is the last element).  Terminates program if index is invalid.
 */

static size_t ArrayCheckIndex(ArrayT *self, ssize_t index) {
  ASSERT((index < self->size) && (index <= -self->size),
         "Index %d out of bound.", index);

  return (index < 0) ? (self->size - index) : index;
}

PtrT ArrayGet(ArrayT *self asm("a0"), ssize_t index asm("d0")) {
  return ArrayGetFast(self, ArrayCheckIndex(self, index));
}

void ArraySet(ArrayT *self asm("a0"), ssize_t index asm("d0"),
              PtrT data asm("a1"))
{
  memcpy(ArrayGet(self, index), data, self->elemSize);
}

void ArrayForEach(ArrayT *self, IterFuncT func, PtrT data) {
  PtrT item = ArrayGetFast(self, 0);
  size_t index = 0;

  while (index++ < self->size) {
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
  ArrayMoveRange(self, first, last + 1, self->size - 1);
  ArrayClearRange(self, self->size - count, self->size - 1);
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
  PtrT last = ArrayGetFast(self, self->size - 1);

  if (self->freeFunc)
      self->freeFunc(item);

  if (item != last)
    memcpy(item, last, self->elemSize);

  if (self->zeroed)
    bzero(last, self->elemSize);
}

void ArrayFilterFast(ArrayT *self, PredicateT func) {
  PtrT item = ArrayGetFast(self, 0);
  size_t index = 0;

  while (index < self->size) {
    if (func(item)) {
      item += self->elemSize;
      index++;
    } else {
      ArrayRemoveFast(self, index);
    }
  }
}

PtrT ArrayInsert(ArrayT *self, ssize_t index) {
  index = ArrayCheckIndex(self, index);

  ArrayMaybeGrow(self, 1);
  ArrayMoveRange(self, index + 1, index, self->size - 2);
  return ArrayGetFast(self, index);
}

PtrT ArrayInsertFast(ArrayT *self, ssize_t index) {
  ArrayMaybeGrow(self, 1);

  {
    PtrT item = ArrayGetFast(self, 0);
    PtrT last = ArrayGetFast(self, self->size - 1);

    memcpy(last, item, self->elemSize);

    if (self->zeroed)
      bzero(item, self->elemSize);

    return item;
  }
}

void ArrayInsertElements(ArrayT *self, ssize_t index, PtrT data, size_t count) {
  index = ArrayCheckIndex(self, index);

  ArrayMaybeGrow(self, count);
  ArrayMoveRange(self, index + count, index, self->size - count - 1);
  memcpy(ArrayGetFast(self, index), data, count * self->elemSize);
}

PtrT ArrayAppend(ArrayT *self) {
  ArrayMaybeGrow(self, 1);

  return ArrayGetFast(self, self->size - 1);
}

void ArrayAppendElements(ArrayT *self, PtrT data, size_t count) {
  ArrayMaybeGrow(self, count);

  {
    PtrT last = ArrayGetFast(self, self->size - count);
    memcpy(last, data, count * self->elemSize);
  }
}

void ArrayInsertionSort(ArrayT *self, ssize_t begin, ssize_t end) {
  CompareFuncT cmp = self->compareFunc;

  begin = ArrayCheckIndex(self, begin);
  end = ArrayCheckIndex(self, end);

  ASSERT(self->compareFunc, "Compare function not set!");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!", begin, end);

  {
    PtrT left = ArrayGetFast(self, begin);
    PtrT right = ArrayGetFast(self, end);
    PtrT tmp = ArrayGetFast(self, self->size);

    size_t elemSize = self->elemSize;

    PtrT pivot = left + elemSize;

    while (pivot <= right) {
      PtrT insert = left;

      while ((insert < pivot) && (cmp(insert, pivot) > 0))
        insert += elemSize;

      if (insert < pivot) {
        memcpy(tmp, pivot, elemSize);
        memmove(insert + elemSize, insert, pivot - insert);
        memcpy(insert, tmp, elemSize);
      }

      pivot += elemSize;
    }
  }
}

size_t ArrayPartition(ArrayT *self, size_t begin, size_t end, PtrT pivot) {
  CompareFuncT cmp = self->compareFunc;
  PtrT left = ArrayGetFast(self, begin);
  PtrT right = ArrayGetFast(self, end);
  size_t elemSize = self->elemSize;
  size_t partition = begin;

  while (left < right) {
    while ((cmp(left, pivot) < 0) && (left < right)) {
      left += elemSize;
      partition++;
    }

    while ((cmp(pivot, right) < 0) && (left < right))
      right -= elemSize;

    ArraySwapFast(self, left, right);
    left += elemSize;
    partition++;
    right -= elemSize;
  }

  return partition;
}

static void QuickSort(ArrayT *self, size_t l, size_t r) {
  while (l < r) {
    PtrT pivot = ArrayGetFast(self, (l + r) / 2);
    size_t i = ArrayPartition(self, l, r, pivot);

    if (i - l <= r - i) {
      ArrayQuickSort(self, l, i);
      l = i + 1;
    } else {
      ArrayQuickSort(self, i + 1, r);
      r = i;
    }
  }
}

void ArrayQuickSort(ArrayT *self, ssize_t begin, ssize_t end) {
  begin = ArrayCheckIndex(self, begin);
  end = ArrayCheckIndex(self, end);

  ASSERT(self->compareFunc, "Compare function not set!");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!", begin, end);

  QuickSort(self, begin, end);
}
