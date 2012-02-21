#include "std/atompool.h"
#include "std/list.h"
#include "std/memory.h"

typedef struct Node {
  struct Node *prev;
  struct Node *next;
  void *item;
} NodeT;

struct List {
  NodeT *first;
  NodeT *last;
  int items;

  AtomPoolT *pool;
};

ListT *NewList() {
  ListT *list = NEW_S(ListT);

  list->pool = NewAtomPool(sizeof(NodeT), 32);
  
  return list;
}

void ResetList(ListT *list) {
  ResetAtomPool(list->pool);

  list->first = NULL;
  list->last = NULL;
  list->items = 0;
}

void DeleteList(ListT *list) {
  if (list) {
    DeleteAtomPool(list->pool);
    DELETE(list);
  }
}

void *ListForEach(ListT *list, IterFuncT func, void *data) {
  NodeT *node = list->first;

  while (node) {
    if (!func(node->item, data))
      break;

    node = node->next;
  }

  return node ? node->item : NULL;
}

void *ListGetNth(ListT *list, size_t index) {
  if (index < list->items) {
    NodeT *node = list->first;

    while (index--)
      node = node->next;

    return node->item;
  }

  return NULL;
}

static NodeT *NodePopBack(ListT *list) {
  NodeT *node = list->last;

  if (node) {
    if (list->first == node)
      list->first = NULL;
    else
      node->prev->next = NULL;

    list->last = node->prev;
    list->items--;
  }

  return node;
}

static NodeT *NodePopFront(ListT *list) {
  NodeT *node = list->first;

  if (node) {
    if (list->last == node)
      list->last = NULL;
    else
      node->next->prev = NULL;

    list->first = node->next;
    list->items--;
  }

  return node;
}

static void NodePushBack(ListT *list, NodeT *node) {
  node->prev = list->last;
  node->next = NULL;

  if (!list->first)
    list->first = node;

  list->last = node;
  list->items++;
}

static void NodePushFront(ListT *list, NodeT *node) {
  node->prev = NULL;
  node->next = list->first;
  
  if (!list->last)
    list->last = node;

  list->first = node;
  list->items++;
}

void *ListPopBack(ListT *list) {
  NodeT *node = NodePopBack(list);

  void *item = (node) ? (node->item) : NULL;

  AtomFree(list->pool, node);

  return item;
}

void *ListPopFront(ListT *list) {
  NodeT *node = NodePopFront(list);

  void *item = (node) ? (node->item) : NULL;

  AtomFree(list->pool, node);

  return item;
}

void ListPushBack(ListT *list, void *item) {
  NodeT *node = AtomNew(list->pool);

  node->item = item;

  NodePushBack(list, node);
}

void ListPushFront(ListT *list, void *item) {
  NodeT *node = AtomNew(list->pool);

  node->item = item;

  NodePushFront(list, node);
}

size_t ListSize(ListT *list) {
  return list->items;
}
