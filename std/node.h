#ifndef __STD_NODE_H__
#define __STD_NODE_H__

#include "std/types.h"

typedef struct _Node NodeT;

struct _Node {
  NodeT *prev;
  NodeT *next;
};

void NodeInitGuard(NodeT *guard);
void NodePrepend(NodeT *cursor, NodeT *node);
void NodeAppend(NodeT *cursor, NodeT *node);
NodeT *NodeUnlink(NodeT *node);
void NodeForEach(NodeT *guard, IterFuncT func, PtrT data);
NodeT *NodeSearch(NodeT *guard, SearchFuncT func, PtrT data);
NodeT *NodeGet(NodeT *guard, ssize_t index);
size_t NodeCount(NodeT *guard);

bool IsGuardEmpty(NodeT *node);

#endif
