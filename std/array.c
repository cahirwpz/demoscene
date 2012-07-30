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

/*
 * Helper functions.
 */

static void MaybeGrow(ArrayT *self, size_t count);
static void MaybeShrink(ArrayT *self, size_t count);

static inline void ClearRange(ArrayT *self, size_t begin, size_t end) {
  if (self->zeroed)
    bzero(ArrayGetFast(self, begin), (end - begin + 1) * self->elemSize);
}

static inline void FreeRange(ArrayT *self, size_t begin, size_t end) {
  if (self->freeFunc)
    ArrayForEachInRange(self, begin, end, (IterFuncT)self->freeFunc, NULL);
}

static inline void MoveRange(ArrayT *self,
                             size_t to, size_t first, size_t last)
{
  size_t remainding = last - first + 1;

  memmove(ArrayGetFast(self, to),
          ArrayGetFast(self, first),
          self->elemSize * remainding);
}

static void RemoveRange(ArrayT *self asm("a0"),
                        size_t first asm("d0"), size_t last asm("d1"))
{
  size_t count = last - first + 1;

  FreeRange(self, first, last);
  MoveRange(self, first, last + 1, self->size - 1);
  ClearRange(self, self->size - count, self->size - 1);
  MaybeShrink(self, count);
}

static inline void RemoveFast(ArrayT *self, PtrT item) {
  PtrT last = ArrayGetFast(self, self->size - 1);

  if (self->freeFunc)
    self->freeFunc(item);

  if (item != last)
    memcpy(item, last, self->elemSize);

  if (self->zeroed)
    bzero(last, self->elemSize);

  self->size--;
}

/*
 * @brief Check if index is valid and normalise it.
 *
 * Accepts negative indices which point to elements starting from the end of
 * array (eg. -1 is the last element).  Terminates program if index is invalid.
 */

static size_t CheckIndex(ArrayT *self asm("a0"), ssize_t index asm("d0")) {
  ASSERT((index < self->size) && (index <= -self->size),
         "Index %d out of bound.", index);

  return (index < 0) ? (self->size - index) : index;
}

/*
 * Getter & setter & swapper.
 */

PtrT ArrayGet(ArrayT *self asm("a0"), ssize_t index asm("d0")) {
  return ArrayGetFast(self, CheckIndex(self, index));
}

void ArraySet(ArrayT *self asm("a0"), ssize_t index asm("d0"),
              PtrT data asm("a1"))
{
  memcpy(ArrayGet(self, index), data, self->elemSize);
}

/*
 * @brief Swap two elements without extra checks.
 *
 * Assumes that fst != snd.
 */

void ArraySwapFast(ArrayT *self asm("a0"), PtrT fst asm("a1"), PtrT snd asm("a2")) {
  PtrT tmp = self->temporary;
  size_t elemSize = self->elemSize;

  memcpy(tmp, fst, elemSize);
  memcpy(fst, snd, elemSize);
  memcpy(snd, tmp, elemSize);
}

/*
 * Iteration functions.
 */

void ArrayForEach(ArrayT *self, IterFuncT func, PtrT data) {
  ArrayForEachInRange(self, 0, self->size - 1, func, data);
}

void ArrayForEachInRange(ArrayT *self, ssize_t begin, ssize_t end,
                         IterFuncT func, PtrT data)
{
  begin = CheckIndex(self, begin);
  end = CheckIndex(self, end);

  ASSERT(begin <= end, "Invalid range of elements specified [%d..%d]!",
         begin, end);

  {
    PtrT item = ArrayGetFast(self, begin);
    PtrT last = ArrayGetFast(self, end);

    do {
      func(item, data);
      item += self->elemSize;
    } while (item <= last);
  }
}

/*
 * Element adding functions.
 */

PtrT ArrayInsertFast(ArrayT *self, ssize_t index, PtrT data) {
  MaybeGrow(self, 1);

  {
    PtrT item = ArrayGetFast(self, 0);
    PtrT last = ArrayGetFast(self, self->size - 1);
    size_t elemSize = self->elemSize;

    memcpy(last, item, elemSize);
    
    if (data)
      memcpy(item, data, elemSize);
    else if (self->zeroed)
      bzero(item, elemSize);

    return item;
  }
}

PtrT ArrayInsert(ArrayT *self, ssize_t index, PtrT data) {
  return ArrayInsertElements(self, index, data, 1);
}

PtrT ArrayInsertElements(ArrayT *self, ssize_t index,
                         PtrT data, size_t count)
{
  PtrT item;

  index = CheckIndex(self, index);
  MaybeGrow(self, count);
  MoveRange(self, index + count, index, self->size - count - 1);

  item = ArrayGetFast(self, self->size - count);

  if (data)
    memcpy(item, data, count * self->elemSize);

  return item;
}

PtrT ArrayAppend(ArrayT *self, PtrT data) {
  return ArrayAppendElements(self, data, 1);
}

PtrT ArrayAppendElements(ArrayT *self, PtrT data, size_t count) {
  PtrT item;

  MaybeGrow(self, count);

  item = ArrayGetFast(self, self->size - count);

  if (data)
    memcpy(item, data, count * self->elemSize);

  return item;
}

/*
 * Element removal functions.
 */

void ArrayRemoveFast(ArrayT *self, size_t index) {
  PtrT item = ArrayGetFast(self, index);

  RemoveFast(self, item);
}

void ArrayRemove(ArrayT *self, ssize_t index) {
  ArrayRemoveRange(self, index, index);
}

void ArrayRemoveRange(ArrayT *self, ssize_t first, ssize_t last) {
  first = CheckIndex(self, first);
  last = CheckIndex(self, last);

  ASSERT(first <= last, "Invalid range of elements specified [%d..%d]!",
         first, last);

  RemoveRange(self, first, last);
}

void ArrayFilterFast(ArrayT *self, PredicateT func) {
  PtrT item = ArrayGetFast(self, 0);
  PtrT last = ArrayGetFast(self, self->size - 1);

  while (item <= last) {
    if (func(item)) {
      item += self->elemSize;
    } else {
      RemoveFast(self, item);
    }
  }
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
  size_t elemSize = self->elemSize;
  size_t bytes = (newSize + 1) * elemSize;

  if (self->data) {
    if (newSize == self->size)
      return;

    if (newSize < self->size) {
      FreeRange(self, newSize, self->size - 1);
      self->size = newSize;
    }
  }

  if (!self->data) {
    self->data = MemNew(bytes, NULL);
    self->size = 0;
  } else {
    PtrT oldData = self->data;
    self->data = MemDupGC(oldData, bytes, NULL);
    MemUnref(oldData);
  }

  if (newSize > self->reserved)
    ClearRange(self, self->reserved, newSize);

  self->reserved = newSize;
  self->temporary = &self->data[newSize * elemSize];
}

#define MIN_SIZE 16

static inline size_t NearestPow2(size_t num) {
  size_t i = 1;

  while (num > i)
    i += i;

  return max(i, MIN_SIZE);
}

/*
 * @brief Check if array can accomodate extra elements and resize if needed.
 */
static void MaybeGrow(ArrayT *self, size_t count) {
  if (self->size + count <= self->reserved) {
    self->size += count;
  } else {
    ArrayResize(self, NearestPow2(self->size));
  }
}

/*
 * @brief Check if array doesn't have too much of an extra space and shrink.
 */
static void MaybeShrink(ArrayT *self, size_t count) {
  self->size -= count;
}

/*
 * Sorting functions.
 */

void ArrayInsertionSort(ArrayT *self, ssize_t begin, ssize_t end) {
  CompareFuncT cmp = self->compareFunc;

  begin = CheckIndex(self, begin);
  end = CheckIndex(self, end);

  ASSERT(cmp, "Compare function not set!");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!",
         begin, end);

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

  ASSERT(cmp, "Compare function not set!");
  ASSERT(left < right, "Invalid range of elements specified [%d..%d]!",
         (int)left, (int)right);

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

static void QuickSort(ArrayT *self asm("a0"),
                      size_t left asm("d0"), size_t right asm("d1"))
{
  while (left < right) {
    PtrT pivot = ArrayGetFast(self, (left + right) / 2);
    size_t i = ArrayPartition(self, left, right, pivot);

    if (i - left <= right - i) {
      QuickSort(self, left, i);
      left = i + 1;
    } else {
      QuickSort(self, i + 1, right);
      right = i;
    }
  }
}

void ArrayQuickSort(ArrayT *self, ssize_t begin, ssize_t end) {
  begin = CheckIndex(self, begin);
  end = CheckIndex(self, end);

  ASSERT(self->compareFunc, "Compare function not set!");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!",
         begin, end);

  QuickSort(self, begin, end);
}
