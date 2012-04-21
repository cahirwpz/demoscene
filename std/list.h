#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include "std/types.h"

typedef struct Node NodeT;

struct Node {
  NodeT *prev;
  NodeT *next;
  PtrT data;
};

NodeT *NewNode(PtrT data);
void NodePrepend(NodeT *cursor, NodeT *node);
void NodeAppend(NodeT *cursor, NodeT *node);
NodeT *NodeUnlink(NodeT *node);
void NodeForEach(NodeT *guard, IterFuncT iterator, PtrT data);
NodeT *NodeSearch(NodeT *guard, SearchFuncT func, PtrT data);

NodeT *NewList();
void ResetList(NodeT *guard);
void DeleteList(NodeT *guard);
void DeleteListFull(NodeT *guard, FreeFuncT deleter);

void ListForEach(NodeT *guard, IterFuncT iterator, PtrT data);
PtrT ListSearch(NodeT *guard, SearchFuncT func, PtrT data);
PtrT ListRemove(NodeT *guard, SearchFuncT func, PtrT data);

NodeT *ListGetNth(NodeT *guard, ssize_t index);
PtrT ListPopBack(NodeT *guard);
PtrT ListPopFront(NodeT *guard);
void ListPushBack(NodeT *guard, PtrT data);
void ListPushFront(NodeT *guard, PtrT data);
size_t ListSize(NodeT *guard);

#endif
