#include "std/debug.h"
#include "std/node.h"
#include "std/memory.h"

void NodeInitGuard(NodeT *guard) {
  guard->next = guard;
  guard->prev = guard;
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
    NodeT *next = node->next;

    func(node, data);
    node = next;
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
