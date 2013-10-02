#define QUICKSORT(TYPE, LESS) \
__regargs static void                                                     \
InsertionSort ## TYPE(TYPE **table, size_t left, size_t right)            \
{                                                                         \
  size_t pivot = left + 1;                                                \
                                                                          \
  while (pivot <= right) {                                                \
    size_t insert = left;                                                 \
                                                                          \
    while ((insert < pivot) && LESS(table[insert], table[pivot]))         \
      insert++;                                                           \
                                                                          \
    if (insert < pivot) {                                                 \
      TYPE **ptr = &table[pivot];                                         \
      size_t n = pivot - insert;                                          \
      TYPE *tmp = *ptr;                                                   \
                                                                          \
      do {                                                                \
        ptr[0] = ptr[-1];                                                 \
        ptr--;                                                            \
      } while (--n > 0);                                                  \
                                                                          \
      *ptr = tmp;                                                         \
    }                                                                     \
                                                                          \
    pivot++;                                                              \
  }                                                                       \
}                                                                         \
                                                                          \
static inline void Swap ## TYPE(TYPE **table, size_t i, size_t j) {       \
  if (i != j) {                                                           \
    TYPE *tmp = table[i]; table[i] = table[j]; table[j] = tmp;            \
  }                                                                       \
}                                                                         \
                                                                          \
__regargs static size_t                                                   \
Partition ## TYPE(TYPE **table, size_t begin, size_t end, TYPE *pivot)    \
{                                                                         \
  size_t left = begin;                                                    \
  size_t right = end;                                                     \
                                                                          \
  while (left < right) {                                                  \
    while ((left < right) && LESS(table[left], pivot))                    \
      left++;                                                             \
                                                                          \
    while ((left < right) && !LESS(table[right], pivot))                  \
      right--;                                                            \
                                                                          \
    if (left < right)                                                     \
      Swap ## TYPE(table, left, right);                                   \
  }                                                                       \
                                                                          \
  return left;                                                            \
}                                                                         \
                                                                          \
__regargs static size_t                                                   \
ChoosePivot ## TYPE(TYPE **table, size_t left, size_t right)              \
{                                                                         \
  size_t middle = (left + right) / 2;                                     \
  TYPE *a = table[left];                                                  \
  TYPE *b = table[middle];                                                \
  TYPE *c = table[right];                                                 \
  size_t pivot;                                                           \
                                                                          \
  if (LESS(a, b)) {                                                       \
    if (LESS(a, c))                                                       \
      pivot = LESS(b, c) ? middle : right;                                \
    else                                                                  \
      pivot = left;                                                       \
  } else {                                                                \
    if (LESS(b, c))                                                       \
      pivot = LESS(a, c) ? left : right;                                  \
    else                                                                  \
      pivot = middle;                                                     \
  }                                                                       \
                                                                          \
  return pivot;                                                           \
}                                                                         \
                                                                          \
static void QuickSort ## TYPE(TYPE **table, size_t left, size_t right) {  \
  if (left < right) {                                                     \
    if (right - left < 6) {                                               \
      InsertionSort ## TYPE(table, left, right);                          \
    } else {                                                              \
      size_t pivot =                                                      \
        ChoosePivot ## TYPE(table, left, right);                          \
      size_t split =                                                      \
        Partition ## TYPE(table, left, right, table[pivot]);              \
                                                                          \
      if (left == split) {                                                \
        Swap ## TYPE(table, left, pivot);                                 \
        split++;                                                          \
      } else {                                                            \
        QuickSort ## TYPE(table, left, split - 1);                        \
      }                                                                   \
                                                                          \
      QuickSort ## TYPE(table, split, right);                             \
    }                                                                     \
  }                                                                       \
}
