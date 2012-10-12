#ifndef __STD_TABLE_H__
#define __STD_TABLE_H__

#include "std/types.h"

typedef bool (*LessFuncT)(const PtrT lhs asm("a0"), const PtrT rhs asm("a1"));

PtrT *NewTableAdapter(PtrT table);

size_t TablePartition(PtrT *table, LessFuncT less,
                      size_t begin, size_t end, PtrT pivot);
void TableSort(PtrT *table, LessFuncT less, size_t begin, size_t end);

#endif
