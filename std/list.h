#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include "std/types.h"

typedef struct List ListT;
typedef struct Node NodeT;

ListT *NewList();
void DeleteList(ListT *list);
void DeleteListFull(ListT *list, FreeFuncT func);
void ResetList(ListT *list);

PtrT ListGetNth(ListT *list, ssize_t index);
void ListForEach(ListT *list, IterFuncT func, PtrT data);
PtrT ListPopBack(ListT *list);
PtrT ListPopFront(ListT *list);
void ListPushBack(ListT *list, PtrT item);
void ListPushFront(ListT *list, PtrT item);
size_t ListSize(ListT *list);
PtrT ListSearch(ListT *list, SearchFuncT func, PtrT data);
PtrT ListRemove(ListT *list, SearchFuncT func, PtrT data);

#endif
