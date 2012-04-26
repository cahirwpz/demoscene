#ifndef __STD_ARRAY_H__
#define __STD_ARRAY_H__

#include "std/types.h"

typedef struct _Array ArrayT;

ArrayT *NewArray(size_t size, size_t elemSize, bool zeroed);
ArrayT *ArraySetFreeFunc(ArrayT *self, FreeFuncT func);
ArrayT *ArrayResize(ArrayT *self, size_t newSize);

PtrT ArrayGet(ArrayT *self, ssize_t index);
size_t ArrayGetElementSize(ArrayT *self);
ArrayT *ArrayAppendElements(ArrayT *self, PtrT data, size_t count);
void ArraySort(ArrayT *self, CompareFuncT func);

#endif
