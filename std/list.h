#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include "std/types.h"
#include "std/node.h"

typedef struct _List ListT;

ListT *NewList();

void ListForEach(ListT *list, IterFuncT func, PtrT data);
void ListFilter(ListT *list, PredicateT func);
PtrT ListSearch(ListT *list, CompareFuncT func, PtrT data);
PtrT ListRemove(ListT *list, CompareFuncT func, PtrT data);

PtrT ListGet(ListT *list, ssize_t index);
PtrT ListPop(ListT *list, ssize_t index);
void ListInsertAt(ListT *list, PtrT data, ssize_t index);
size_t ListSize(ListT *list);

PtrT ListPopBack(ListT *list);
PtrT ListPopFront(ListT *list);
void ListPushBack(ListT *list, PtrT data);
void ListPushFront(ListT *list, PtrT data);

#endif
