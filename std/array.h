#ifndef __STD_ARRAY_H__
#define __STD_ARRAY_H__

#include "std/types.h"

typedef struct _Array ArrayT;

typedef struct _ArrayData {
  size_t size;
  uint8_t data[0];
} ArrayDataT;

ArrayT *NewArray(size_t size, size_t elemSize, bool zeroed);
ArrayT *NewPtrArray(size_t reserved, bool autoFree);
void ArraySetCompareFunc(ArrayT *self, CompareFuncT func);
void ArraySetFreeFunc(ArrayT *self, FreeFuncT func);
void ArrayResize(ArrayT *self, size_t newSize);

/*
 * Use with caution! After the size of the array is changed a pointer obtained
 * with this function becomes invalid!
 */
ArrayDataT *ArrayGetData(ArrayT *self asm("a0"));

PtrT ArrayGet(ArrayT *self asm("a0"), ssize_t index asm("d0"));
PtrT ArrayGetFast(ArrayT *self asm("a0"), size_t index asm("d0"));
void ArraySet(ArrayT *self asm("a0"), ssize_t index asm("d0"), PtrT data asm("a1"));
void ArrayForEach(ArrayT *self, IterFuncT func, PtrT data);

void ArrayRemove(ArrayT *self, ssize_t index);
void ArrayRemoveRange(ArrayT *self, ssize_t first, ssize_t last);
void ArrayRemoveFast(ArrayT *self, size_t index);

void ArrayFilterFast(ArrayT *self, PredicateT func);

PtrT ArrayInsert(ArrayT *self, ssize_t index);
PtrT ArrayInsertFast(ArrayT *self, ssize_t index);
void ArrayInsertElements(ArrayT *self, ssize_t index, PtrT data, size_t count);
PtrT ArrayAppend(ArrayT *self);
void ArrayAppendElements(ArrayT *self, PtrT data, size_t count);

void ArrayInsertionSort(ArrayT *self, ssize_t begin, ssize_t end);
size_t ArrayPartition(ArrayT *self, size_t begin, size_t end, PtrT pivot);
void ArrayQuickSort(ArrayT *self, ssize_t begin, ssize_t end);

#endif
