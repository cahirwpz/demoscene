#include "std/debug.h"
#include "std/node.h"
#include "std/memory.h"

void NodeInitGuard(NodeT *guard) {
  guard->next = guard;
  guard->prev = guard;
}

void NodeAppend(NodeT *cursor, NodeT *node) {
  node->prev = cursor;
  node->next = cursor->next;
  
  cursor->next->prev = node;
  cursor->next = node;
}

void NodePrepend(NodeT *cursor, NodeT *node) {
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
    NodeT *next = node->next;

    func(node, data);
    node = next;
  }
}

NodeT *NodeSearch(NodeT *guard, CompareFuncT func, PtrT data) {
  NodeT *node = guard->next;

  while (node != guard) {
    if (func(node, data) == CMP_EQ)
      break;

    node = node->next;
  }

  return (node != guard) ? node : NULL;
}

NodeT *NodeGet(NodeT *guard, ssize_t index) {
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

  return (node != guard) ? node : NULL;
}

size_t NodeCount(NodeT *guard) {
  NodeT *node = guard->next;
  size_t size = 0;

  while (node != guard) {
    node = node->next;
    size++;
  }

  return size;
}

bool IsGuardEmpty(NodeT *node) {
  return (node->prev == node->next);
}
