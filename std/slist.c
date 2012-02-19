#include "std/memory.h"
#include "std/slist.h"

SListT *NewSList() {
  return NEW_S(SListT);
}

void DeleteSList(SListT *list, FreeFuncT func) {
  if (list) {
    SNodeT *node = list->first;

    while (node) {
      SNodeT *next = node->next;

      if (func)
        func(node->item);

      DELETE(node);

      node = next;
    }

    DELETE(list);
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
  if (index < list->items) {
    SNodeT *node = list->first;

    while (index--)
      node = node->next;

    return node->item;
  }

  return NULL;
}

void SL_PushFrontNode(SListT *list, SNodeT *node) {
  node->next = list->first;
  
  if (!list->first)
    list->last = node;

  list->first = node;
  list->items++;
}

void SL_PushFront(SListT *list, void *item) {
  SNodeT *node = NEW_S(SNodeT);

  node->item = item;

  SL_PushFrontNode(list, node);
}

SNodeT *SL_PopFrontNode(SListT *list) {
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
  SNodeT *node = SL_PopFrontNode(list);

  void *item = (node) ? (node->item) : NULL;

  DELETE(node);

  return item;
}
