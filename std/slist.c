#include "system/memory.h"
#include "std/slist.h"

SListT *NewSList() {
  return NEW_SZ(SListT);
}

void DeleteSList(SListT *list, FreeFuncT func) {
  if (!list)
    return;

  SNodeT *node = list->first;

  while (node) {
    SNodeT *next = node->next;

    if (func)
      func(node->item);

    DELETE(node);

    node = next;
  }
}

void SL_Concat(SListT *dst, SListT *src) {
  if (src->first) {
    if (dst->first) {
      dst->last->next = src->first;
      dst->last = src->last;
    } else {
      dst->first = src->first;
      dst->last = src->last;
    }

    dst->items += src->items;

    src->first = NULL;
    src->last = NULL;
    src->items = 0;
  }
}

void *SL_GetNth(SListT *list, size_t index) {
  if (index > list->items)
    return NULL;

  SNodeT *node = list->first;

  while (index--)
    node = node->next;

  return node->item;
}

void SL_PushFrontLink(SListT *list, SNodeT *node) {
  node->next = list->first;
  
  if (!list->first)
    list->last = node;

  list->first = node;
  list->items++;
}

bool SL_PushFront(SListT *list, void *item) {
  SNodeT *node = NEW_SZ(SNodeT);

  if (node) {
    node->item = item;

    SL_PushFrontLink(list, node);
  }

  return (node) ? TRUE : FALSE;
}

SNodeT *SL_PopFrontLink(SListT *list) {
  SNodeT *node = list->first;

  if (node) {
    if (list->first == list->last)
      list->last = NULL;

    list->first = node->next;
    list->items--;
  }

  return node;
}

void *SL_PopFront(SListT *list) {
  SNodeT *node = SL_PopFrontLink(list);

  void *item = (node) ? (node->item) : NULL;

  DELETE(node);

  return item;
}
