#include "std/memory.h"
#include "std/slist.h"
#include "std/atompool.h"

typedef struct SNode {
  struct SNode *next;
  void *item;
} SNodeT;

struct SList {
  SNodeT *first;
  SNodeT *last;
  int items;

  AtomPoolT *pool;
};

SListT *NewSList() {
  SListT *list = NEW_S(SListT);

  list->pool = NewAtomPool(sizeof(SNodeT), 32);
  
  return list;
}

void ResetSList(SListT *list) {
  ResetAtomPool(list->pool);

  list->first = NULL;
  list->last = NULL;
  list->items = 0;
}

void DeleteSList(SListT *list) {
  if (list) {
    DELETE(list->pool);
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

void SL_ForEach(SListT *list, IterFuncT func, void *data) {
  SNodeT *node = list->first;

  while (node) {
    if (!func(node->item, data))
      break;

    node = node->next;
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

static void SL_PushFrontNode(SListT *list, SNodeT *node) {
  node->next = list->first;
  
  if (!list->first)
    list->last = node;

  list->first = node;
  list->items++;
}

static SNodeT *SL_PopFrontNode(SListT *list) {
  SNodeT *node = list->first;

  if (node) {
    if (list->first == list->last)
      list->last = NULL;

    list->first = node->next;
    list->items--;
  }

  return node;
}

void SL_PushFront(SListT *list, void *item) {
  SNodeT *node = AtomNew(list->pool);

  node->item = item;

  SL_PushFrontNode(list, node);
}

void *SL_PopFront(SListT *list) {
  SNodeT *node = SL_PopFrontNode(list);

  void *item = (node) ? (node->item) : NULL;

  AtomFree(list->pool, node);

  return item;
}

size_t SL_Size(SListT *list) {
  return list->items;
}
