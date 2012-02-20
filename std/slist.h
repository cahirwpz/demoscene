#ifndef __STD_SLIST_H__
#define __STD_SLIST_H__

#include "std/types.h"

typedef struct SList SListT;

SListT *NewSList();
void DeleteSList(SListT *list);
void ResetSList(SListT *list);

void *SL_GetNth(SListT *list, size_t index);
void *SL_ForEach(SListT *list, IterFuncT func, void *data);
void *SL_PopFront(SListT *list);
void SL_PushFront(SListT *list, void *item);
size_t SL_Size(SListT *list);

#endif
