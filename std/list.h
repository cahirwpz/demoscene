#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include "std/types.h"
#include "std/node.h"

typedef struct List {
  NodeT node;
  PtrT data;
} ListT;

ListT *NewList();
void ResetList(ListT *list);
void DeleteList(ListT *list);
void DeleteListFull(ListT *list, FreeFuncT deleter);

void ListForEach(ListT *list, IterFuncT func, PtrT data);
PtrT ListSearch(ListT *list, SearchFuncT func, PtrT data);
PtrT ListRemove(ListT *list, SearchFuncT func, PtrT data);

PtrT ListGetNth(ListT *list, ssize_t index);
PtrT ListPopBack(ListT *list);
PtrT ListPopFront(ListT *list);
void ListPushBack(ListT *list, PtrT data);
void ListPushFront(ListT *list, PtrT data);
size_t ListSize(ListT *list);

#endif
