#include "std/debug.h"
#include "std/list.h"
#include "std/memory.h"

struct Node {
  NodeT *prev;
  NodeT *next;
  PtrT item;
};

struct List {
  NodeT *first;
  NodeT *last;
  int items;
};

ListT *NewList() {
  return NEW_S(ListT);
}

void ResetList(ListT *list) {
  ListForEachNode(list, (IterFuncT)MemFree, NULL);

  list->first = NULL;
  list->last = NULL;
  list->items = 0;
}

void DeleteList(ListT *list) {
  if (list) {
    ListForEachNode(list, (IterFuncT)MemFree, NULL);
    DELETE(list);
  }
}

void DeleteListFull(ListT *list, FreeFuncT delete) {
  if (list) {
    ListForEach(list, (IterFuncT)delete, NULL);
    ListForEachNode(list, (IterFuncT)MemFree, NULL);
    DELETE(list);
  }
}

void ListForEachNode(ListT *list, IterFuncT func, PtrT data) {
  NodeT *node = list->first;

  while (node) {
    func(node, data);
    node = node->next;
  }
}

void ListForEach(ListT *list, IterFuncT func, PtrT data) {
  NodeT *node = list->first;

  while (node) {
    func(node->item, data);

    node = node->next;
  }
}

PtrT ListGetNth(ListT *list, ssize_t index) {
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

static PtrT NodeGetItem(NodeT *node) {
  return node ? node->item : NULL;
}

static PtrT NodeUnlink(ListT *list, NodeT *node) {
  PtrT item = NULL;

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

    DELETE(node);
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

PtrT ListPopBack(ListT *list) {
  return NodeUnlink(list, list->last);
}

PtrT ListPopFront(ListT *list) {
  return NodeUnlink(list, list->first);
}

void ListPushBack(ListT *list, PtrT item) {
  NodeT *node = NEW_S(NodeT);

  node->item = item;

  NodePushBack(list, node);
}

void ListPushFront(ListT *list, PtrT item) {
  NodeT *node = NEW_S(NodeT);

  node->item = item;

  NodePushFront(list, node);
}

size_t ListSize(ListT *list) {
  return list->items;
}

static NodeT *ListSearchNode(ListT *list, SearchFuncT func, PtrT data) {
  NodeT *node = list->first;

  while (node) {
    if (!func(node->item, data))
      break;

    node = node->next;
  }

  return node;
}

PtrT ListSearch(ListT *list, SearchFuncT func, PtrT data) {
  return NodeGetItem(ListSearchNode(list, func, data));
}

PtrT ListRemove(ListT *list, SearchFuncT func, PtrT data) {
  NodeT *node = ListSearchNode(list, func, data);

  return NodeUnlink(list, node);
}
