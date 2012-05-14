#ifndef __STD_ARRAY_H__
#define __STD_ARRAY_H__

#include "std/types.h"

typedef struct _Array ArrayT;

typedef struct _ArrayData {
  size_t size;
  uint8_t data[0];
} ArrayDataT;

ArrayT *NewArray(size_t size, size_t elemSize, bool zeroed);
void ArraySetFreeFunc(ArrayT *self, FreeFuncT func);
void ArrayResize(ArrayT *self, size_t newSize);

/*
 * Use with caution! After the size of the array is changed a pointer obtained
 * with this function becomes invalid!
 */
ArrayDataT *ArrayGetData(ArrayT *self);

PtrT ArrayGet(ArrayT *self, ssize_t index);
PtrT ArrayGetFast(ArrayT *self, size_t index);
void ArraySet(ArrayT *self, ssize_t index, PtrT data);
void ArrayForEach(ArrayT *self, IterFuncT func, PtrT data);

void ArrayRemove(ArrayT *self, ssize_t index);
void ArrayRemoveRange(ArrayT *self, ssize_t first, ssize_t last);
void ArrayRemoveFast(ArrayT *self, size_t index);

PtrT ArrayInsert(ArrayT *self, ssize_t index);
PtrT ArrayInsertFast(ArrayT *self, ssize_t index);
void ArrayInsertElements(ArrayT *self, ssize_t index, PtrT data, size_t count);
PtrT ArrayAppend(ArrayT *self);
void ArrayAppendElements(ArrayT *self, PtrT data, size_t count);

void ArraySort(ArrayT *self, CompareFuncT func, ssize_t first, ssize_t last);

#endif
