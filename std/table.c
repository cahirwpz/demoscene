#include "std/memory.h"
#include "std/debug.h"
#include "std/table.h"

PtrT TableElemGet(PtrT self asm("a0"), size_t index asm("d0")) {
  return self + index * TableElemSize(self);
}

PtrT *NewTableAdapter(PtrT table) {
  size_t size = TableSize(table);
  size_t elemSize = TableElemSize(table);

  PtrT *adapter = NewTable(PtrT, size);

  size_t i;

  for (i = 0; i < size; i++)
    adapter[i] = table + i * elemSize;

  return adapter;
}

#ifdef DEBUG
static void VerifySort(PtrT *table, LessFuncT less, size_t begin, size_t end) {
  size_t i;

  if (begin == end)
    return;

  LOG("Verify sorted range [%d..%d].", (int)begin, (int)end);

  for (i = begin + 1; i <= end; i++)
    ASSERT(!less(table[i], table[i - 1]), "Fail: %d > %d.", (int)(i - 1), (int)i); 
}
#else
#define VerifySort(table, less, begin, end)
#endif

#ifdef DEBUG
static void VerifyPartition(PtrT *table, LessFuncT less,
                            size_t begin, size_t split, size_t end, PtrT pivot)
{
  size_t i;

  if (begin == end)
    return;

  LOG("Verify partition [%d..%d] [%d..%d].", (int)begin, (int)split - 1, (int)split, (int)end);

  for (i = begin; i < split; i++)
    ASSERT(less(table[i], pivot), "Value of (%d) is bigger or equal to the value of pivot.", (int)i);

  for (i = split; i <= end; i++)
    ASSERT(!less(table[i], pivot), "Value of (%d) is lesser than the value of pivot.", (int)i);
}
#else
#define VerifyPartition(table, less, begin, partition, end, pivot)
#endif

static void InsertionSort(PtrT *table, LessFuncT less,
                          size_t left, size_t right) {
  size_t pivot = left + 1;

  while (pivot <= right) {
    size_t insert = left;

    while ((insert < pivot) && less(table[insert], table[pivot]))
      insert++;

    if (insert < pivot) {
      PtrT *ptr = &table[pivot];
      size_t n = pivot - insert;

      PtrT tmp = *ptr;

      do {
        ptr[0] = ptr[-1];
        ptr--;
      } while (--n > 0);

      *ptr = tmp;
    }

    pivot++;
  }
}

static inline void Swap(PtrT *table, size_t i, size_t j) {
  if (i != j) {
    PtrT tmp = table[i]; table[i] = table[j]; table[j] = tmp;
  }
}

__regargs static size_t Partition(PtrT *table, LessFuncT less,
                                  size_t begin, size_t end, PtrT pivot)
{
  size_t left = begin;
  size_t right = end;

  while (left < right) {
    while ((left < right) && less(table[left], pivot))
      left++;

    while ((left < right) && !less(table[right], pivot))
      right--;

    if (left < right)
      Swap(table, left, right);
  }

  VerifyPartition(table, less, begin, left, end, pivot);

  return left;
}

__regargs static size_t ChoosePivot(PtrT *table, LessFuncT less,
                                    size_t left, size_t right) {
  size_t middle = (left + right) / 2;

  PtrT a = table[left];
  PtrT b = table[middle];
  PtrT c = table[right];

  size_t pivot;

  /* Select middle point. */
  if (less(a, b)) {
    if (less(a, c))
      pivot = less(b, c) ? middle : right;
    else
      pivot = left;
  } else {
    if (less(b, c))
      pivot = less(a, c) ? left : right;
    else
      pivot = middle;
  }

  return pivot;
}

static void QuickSort(PtrT *table, LessFuncT less,
                      size_t left, size_t right)
{
  if (left < right) {
    if (right - left < 6) {
      InsertionSort(table, less, left, right);
    } else {
      size_t pivot = ChoosePivot(table, less, left, right);
      size_t split = Partition(table, less, left, right, table[pivot]);

      if (left == split) {
        Swap(table, left, pivot);
        split++;
      } else {
        QuickSort(table, less, left, split - 1);
        VerifySort(table, less, left, split - 1);
      }

      QuickSort(table, less, split, right);
      VerifySort(table, less, split, right);
    }
  }
}

size_t TablePartition(PtrT *table, LessFuncT less,
                      size_t begin, size_t end, PtrT pivot) {
  ASSERT(begin < TableSize(table), "Begin index out of bounds.");
  ASSERT(end < TableSize(table), "End index out of bounds.");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!",
         (int)begin, (int)end);

  return Partition(table, less, begin, end, pivot);
}

void TableSort(PtrT *table, LessFuncT less, size_t begin, size_t end) {
  ASSERT(begin < TableSize(table), "Begin index out of bounds.");
  ASSERT(end < TableSize(table), "End index out of bounds.");
  ASSERT(begin < end, "Invalid range of elements specified [%d..%d]!",
         (int)begin, (int)end);

  QuickSort(table, less, begin, end);
}
