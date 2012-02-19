#ifndef __STD_SLIST_H__
#define __STD_SLIST_H__

#include "std/types.h"

typedef struct SList SListT;

SListT *NewSList();
void DeleteSList(SListT *list);
void ResetSList(SListT *list);

void SL_Concat(SListT *dst, SListT *src);
void *SL_GetNth(SListT *list, size_t index);
void SL_PushFront(SListT *list, void *item);
void *SL_PopFront(SListT *list);
size_t SL_Size(SListT *list);

#endif
