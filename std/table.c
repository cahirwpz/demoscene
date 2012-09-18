#include "std/memory.h"
#include "std/debug.h"
#include "std/table.h"

#include <string.h>

struct SortAdapter {
  PtrT table;
  size_t elemSize;
  PtrT temporary;
  PtrT pivot;
  LessFuncT lessFunc;
};

static void DeleteSortAdapter(SortAdapterT *self) {
  MemUnref(self->pivot);
  MemUnref(self->temporary);
}

TYPEDECL(SortAdapterT, (FreeFuncT)DeleteSortAdapter);

SortAdapterT *NewSortAdapter(PtrT table, LessFuncT lessFunc) {
  SortAdapterT *adapter = NewInstance(SortAdapterT);

  size_t elemSize = TableElemSize(table);

  adapter->table = table;
  adapter->elemSize = elemSize;
  adapter->temporary = MemNew(elemSize);
  adapter->pivot = MemNew(elemSize);
  adapter->lessFunc = lessFunc;

  return adapter;
}

static inline PtrT GetElem(const SortAdapterT *self, size_t index) {
  return self->table + index * self->elemSize;
}

static void InsertionSort(SortAdapterT *self asm("a0"), int begin asm("d0"), int end asm("d1")) {
  LessFuncT less = self->lessFunc;
  size_t elemSize = self->elemSize;

  PtrT left = GetElem(self, begin);
  PtrT right = GetElem(self, end);
  PtrT tmp = self->temporary;
  PtrT pivot = left + elemSize;

  while (pivot <= right) {
    PtrT insert = left;

    while ((insert < pivot) && less(insert, pivot))
      insert += elemSize;

    if (insert < pivot) {
      memcpy(tmp, pivot, elemSize);
      memmove(insert + elemSize, insert, pivot - insert);
      memcpy(insert, tmp, elemSize);
    }

    pivot += elemSize;
  }
}

static size_t Partition(const SortAdapterT *self asm("a0"),
                        int begin asm("d0"), int end asm("d1"),
                        PtrT pivot asm("a1"))
{
  LessFuncT less = self->lessFunc;
  PtrT left = GetElem(self, begin);
  PtrT right = GetElem(self, end);
  size_t elemSize = self->elemSize;
  size_t partition = begin;
  PtrT tmp = self->temporary;

  while (left < right) {
    while ((left < right) && less(left, pivot)) {
      left += elemSize;
      partition++;
    }

    while ((left < right) && less(pivot, right))
      right -= elemSize;

    if (left < right) {
      memcpy(tmp, left, elemSize);
      memcpy(left, right, elemSize);
      memcpy(right, tmp, elemSize);
    }
  }

#if DEBUG
  LOG("Partition on range [%d..%d] [%d..%d].",
      (int)begin, (int)(partition - 1), (int)(partition), (int)end);
#endif

  return partition;
}

#if DEBUG
static void Verify(SortAdapterT *self, size_t begin, size_t end) {
  int i;

  LOG("Verify range [%d..%d].", (int)begin, (int)end);

  for (i = begin + 1; i <= end; i++)
    ASSERT(self->lessFunc(GetElem(self, i - 1), GetElem(self, i)),
           "Fail: %d > %d.", i - 1, i);
}
#endif

static void ChoosePivot(SortAdapterT *self, int left, int right) {
  PtrT a = GetElem(self, left);
  PtrT b = GetElem(self, (left + right) / 2);
  PtrT c = GetElem(self, right);
  PtrT pivot;

  LessFuncT less = self->lessFunc;

  /* Select middle point. */
  if (less(a, b)) {
    if (less(a, c))
      pivot = less(b, c) ? b : c;
    else
      pivot = a;
  } else {
    if (less(b, c))
      pivot = less(a, c) ? a : c;
    else
      pivot = b;
  }

  memcpy(self->pivot, pivot, self->elemSize);
}

static void QuickSort(SortAdapterT *self asm("a0"),
                      int left asm("d0"), int right asm("d1"))
{
  if (left < right) {
    if (left - right < 6) {
      InsertionSort(self, left, right);
    } else {
      int split;

      ChoosePivot(self, left, right);

      split = Partition(self, left, right, self->pivot);

      QuickSort(self, left, split);
      QuickSort(self, split + 1, right);
    }
  }
}

size_t TablePartition(SortAdapterT *self, size_t begin, size_t end, PtrT pivot)
{
  ASSERT(begin < TableSize(self->table), "Begin index out of bounds.");
  ASSERT(end < TableSize(self->table), "End index out of bounds.");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!",
         (int)begin, (int)end);

  return Partition(self, begin, end, pivot);
}

void TableSort(SortAdapterT *self, size_t begin, size_t end) {
  ASSERT(begin < TableSize(self->table), "Begin index out of bounds.");
  ASSERT(end < TableSize(self->table), "End index out of bounds.");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!",
         (int)begin, (int)end);

  QuickSort(self, begin, end);

#if DEBUG
  Verify(self, begin, end);
#endif
}
