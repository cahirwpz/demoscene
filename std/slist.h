#ifndef __STD_SLIST_H__
#define __STD_SLIST_H__

#include "std/types.h"

typedef struct SNode {
  struct SNode *next;
  void *item;
} SNodeT;

typedef struct SList {
  SNodeT *first;
  SNodeT *last;
  int items;
} SListT;

SListT *NewSList();
void DeleteSList(SListT *list, FreeFuncT func);

void SL_Concat(SListT *dst, SListT *src);
void SL_PushFrontNode(SListT *list, SNodeT *node);
SNodeT *SL_PopFrontNode(SListT *list);

void *SL_GetNth(SListT *list, size_t index);
void SL_PushFront(SListT *list, void *item);
void *SL_PopFront(SListT *list);

#endif
