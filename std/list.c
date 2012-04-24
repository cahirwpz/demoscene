#include "std/debug.h"
#include "std/list.h"
#include "std/memory.h"

static NodeT *GetNode(ListT *list) {
  return list ? (&list->node) : NULL;
}

static PtrT GetData(NodeT *node) {
  return node ? ((ListT *)node)->data : NULL;
}

static NodeT *NodeAlloc(PtrT data) {
  ListT *item = NewRecord(ListT);
  item->data = data;
  return GetNode(item);
}

static PtrT NodeFree(NodeT *node) {
  PtrT data = GetData(node);
  MemUnref(node);
  return data;
}

ListT *NewList() {
  ListT *list = NewRecord(ListT);
  NodeInitGuard(GetNode(list));
  return list;
}

void ResetList(ListT *list) {
  if (list) {
    NodeForEach(GetNode(list), (IterFuncT)MemUnref, NULL);
    NodeInitGuard(GetNode(list));
  }
}

void DeleteList(ListT *list) {
  if (list) {
    NodeForEach(GetNode(list), (IterFuncT)MemUnref, NULL);
    MemUnref(list);
  }
}

void DeleteListFull(ListT *list, FreeFuncT func) {
  if (list) {
    ListForEach(list, (IterFuncT)func, NULL);
    DeleteList(list);
  }
}

void ListForEach(ListT *list, IterFuncT func, PtrT data) {
  NodeT *guard = GetNode(list);
  NodeT *node = guard->next;

  while (node != guard) {
    func(GetData(node), data);
    node = node->next;
  }
}

PtrT ListSearch(ListT *list, CompareFuncT func, PtrT data) {
  NodeT *guard = GetNode(list);
  NodeT *node = guard->next;

  while (node != guard) {
    if (func(GetData(node), data) == CMP_EQ)
      break;

    node = node->next;
  }

  return (node != guard) ? GetData(node) : NULL;
}

PtrT ListRemove(ListT *list, CompareFuncT func, PtrT data) {
  return NodeFree(NodeUnlink(ListSearch(list, func, data)));
}

PtrT ListGet(ListT *list, ssize_t index) {
  return GetData(NodeGet(GetNode(list), index));
}

PtrT ListPop(ListT *list, ssize_t index) {
  return NodeFree(NodeGet(GetNode(list), index));
}

void ListInsertAt(ListT *list, PtrT data, ssize_t index) {
  NodePrepend(NodeGet(GetNode(list), index), NodeAlloc(data));
}

PtrT ListPopBack(ListT *list) {
  return ListPop(list, -1);
}

PtrT ListPopFront(ListT *list) {
  return ListPop(list, 0);
}

void ListPushFront(ListT *list, PtrT data) {
  NodeAppend(GetNode(list), NodeAlloc(data));
}

void ListPushBack(ListT *list, PtrT data) {
  NodePrepend(GetNode(list), NodeAlloc(data));
}

size_t ListSize(ListT *list) {
  return NodeCount(GetNode(list));
}
