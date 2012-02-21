#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include "std/types.h"

typedef struct List ListT;

ListT *NewList();
void DeleteList(ListT *list);
void ResetList(ListT *list);

void *ListGetNth(ListT *list, size_t index);
void *ListForEach(ListT *list, IterFuncT func, void *data);
void *ListPopBack(ListT *list);
void *ListPopFront(ListT *list);
void ListPushBack(ListT *list, void *item);
void ListPushFront(ListT *list, void *item);
size_t ListSize(ListT *list);

#endif
