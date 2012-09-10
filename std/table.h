#ifndef __STD_TABLE_H__
#define __STD_TABLE_H__

#include "std/types.h"

typedef bool (*LessFuncT)(const PtrT lhs asm("a0"), const PtrT rhs asm("a1"));

typedef struct SortAdapter SortAdapterT;

SortAdapterT *NewSortAdapter(PtrT table, LessFuncT cmpFunc);
size_t TablePartition(SortAdapterT *self,
                      size_t begin, size_t end, PtrT pivot);
void TableSort(SortAdapterT *self, size_t begin, size_t end);

#endif
