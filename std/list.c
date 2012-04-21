#include "std/debug.h"
#include "std/list.h"
#include "std/memory.h"

/*
 * Node handling related functions.
 */

NodeT *NewNode(PtrT data) {
  NodeT *node = MemNew0(sizeof(NodeT));

  node->data = data;

  return node;
}

static PtrT NodeGetData(NodeT *node) {
    return node ? node->data : NULL;
}

void NodePrepend(NodeT *cursor, NodeT *node) {
  node->prev = cursor;
  node->next = cursor->next;
  
  cursor->next->prev = node;
  cursor->next = node;
}

void NodeAppend(NodeT *cursor, NodeT *node) {
  node->prev = cursor->prev;
  node->next = cursor;

  cursor->prev->next = node;
  cursor->prev = node;
}

NodeT *NodeUnlink(NodeT *node) {
  if (node) {
    if (node->prev == node->next) {
      node = NULL;
    } else {
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }
  }

  return node;
}

void NodeForEach(NodeT *guard, IterFuncT func, PtrT data) {
  NodeT *node = guard->next;

  while (node != guard) {
    func(node, data);
    node = node->next;
  }
}

NodeT *NodeSearch(NodeT *guard, SearchFuncT func, PtrT data) {
  NodeT *node = guard->next;

  while (node != guard) {
    if (!func(node, data))
      break;

    node = node->next;
  }

  return (node != guard) ? node : NULL;
}

/*
 * List related functions.
 */

NodeT *NewList() {
  NodeT *node = MemNew0(sizeof(NodeT));

  node->next = node;
  node->prev = node;

  return node;
}

void DeleteList(NodeT *guard) {
  if (guard) {
    NodeT *node = guard->next;

    while (node != guard) {
      MemFree(node);
      node = node->next;
    }

    MemFree(guard);
  }
}

void DeleteListFull(NodeT *guard, FreeFuncT deleter) {
  if (guard) {
    ListForEach(guard, (IterFuncT)deleter, NULL);
    DeleteList(guard);
  }
}

void ListForEach(NodeT *guard, IterFuncT iterator, PtrT data) {
  NodeT *node = guard->next;

  while (node != guard) {
    iterator(node->data, data);
    node = node->next;
  }
}

PtrT ListSearch(NodeT *guard, SearchFuncT func, PtrT data) {
  NodeT *node = guard->next;

  while (node != guard) {
    if (!func(node->data, data))
      break;

    node = node->next;
  }

  return (node == guard) ? NULL : NodeGetData(node);
}

static PtrT NodeRelinquish(NodeT *node) {
  PtrT data = NodeGetData(node);
  MemFree(node);
  return data;
}

PtrT ListRemove(NodeT *guard, SearchFuncT func, PtrT data) {
  return NodeRelinquish(NodeUnlink(ListSearch(guard, func, data)));
}

NodeT *ListGetNth(NodeT *guard, ssize_t index) {
  NodeT *node;

  if (index >= 0) {
    node = guard->next;

    while (node != guard && index--)
      node = node->next;
  } else {
    node = guard->prev;

    while (node != guard && ++index < 0)
      node = node->prev;
  }

  return (node == guard) ? NULL : node;
}

PtrT ListPopBack(NodeT *guard) {
  return NodeRelinquish(NodeUnlink(guard->prev));
}

PtrT ListPopFront(NodeT *guard) {
  return NodeRelinquish(NodeUnlink(guard->next));
}

void ListPushFront(NodeT *guard, PtrT data) {
  NodeAppend(guard, NewNode(data));
}

void ListPushBack(NodeT *guard, PtrT data) {
  NodePrepend(guard, NewNode(data));
}

size_t ListSize(NodeT *guard) {
  NodeT *node = guard->next;
  size_t size = 0;

  while (node != guard) {
    node = node->next;
    size++;
  }

  return size;
}
