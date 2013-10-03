#ifndef __STD_TABLE_H__
#define __STD_TABLE_H__

#include "std/types.h"

typedef __regargs bool (*LessFuncT)(const PtrT lhs, const PtrT rhs);

__regargs PtrT TableElemGet(PtrT self, size_t index);
__regargs void TableElemSwap(PtrT self, size_t i, size_t j);

/* Create a table of pointers to the elements of original table. */
PtrT *NewTableAdapter(PtrT table);

size_t TablePartition(PtrT table, LessFuncT less,
                      size_t begin, size_t end, PtrT pivot);
void TableSort(PtrT table, LessFuncT less, size_t begin, size_t end);

#endif
