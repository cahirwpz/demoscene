#include "std/atompool.h"
#include "std/debug.h"
#include "std/list.h"
#include "std/memory.h"

struct Node {
  NodeT *prev;
  NodeT *next;
  void *item;
};

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

void DeleteListFull(ListT *list, FreeFuncT delete) {
  if (list) {
    ListForEach(list, (IterFuncT)delete, NULL);
    DeleteAtomPool(list->pool);
    DELETE(list);
  }
}

void ListForEach(ListT *list, IterFuncT func, void *data) {
  NodeT *node = list->first;

  while (node) {
    func(node->item, data);

    node = node->next;
  }
}

void *ListGetNth(ListT *list, ssize_t index) {
  if ((index < list->items) && (index >= -list->items)) {
    NodeT *node;

    if (index >= 0) {
      node = list->first;

      while (index--)
        node = node->next;
    } else {
      node = list->last;

      while (++index < 0)
        node = node->prev;
    }

    return node->item;
  }

  return NULL;
}

static void *NodeGetItem(NodeT *node) {
  return node ? node->item : NULL;
}

static void* NodeUnlink(ListT *list, NodeT *node) {
  void *item = NULL;

  if (node) {
    if (!node->prev) {
      /* unlink first node */
      if (node->next)
        node->next->prev = NULL;
      else
        list->last = NULL;

      list->first = node->next;
    } else if (!node->next) {
      /* unlink last node */
      if (node->prev)
        node->prev->next = NULL;
      else
        list->first = NULL;

      list->last = node->prev;
    } else {
      /* unlink internal node */
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }

    list->items--;

    item = NodeGetItem(node);

    AtomFree(list->pool, node);
  }

  return item;
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
  return NodeUnlink(list, list->last);
}

void *ListPopFront(ListT *list) {
  return NodeUnlink(list, list->first);
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

static NodeT *ListSearchNode(ListT *list, SearchFuncT func, void *data) {
  NodeT *node = list->first;

  while (node) {
    if (!func(node->item, data))
      break;

    node = node->next;
  }

  return node;
}

void *ListSearch(ListT *list, SearchFuncT func, void *data) {
  return NodeGetItem(ListSearchNode(list, func, data));
}

void *ListRemove(ListT *list, SearchFuncT func, void *data) {
  NodeT *node = ListSearchNode(list, func, data);

  return NodeUnlink(list, node);
}
