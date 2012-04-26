#include "std/memory.h"
#include "std/tree.h"

struct _Tree {
  NodeT node;
  NodeT children;
  TreeT *parent;
  PtrT data;
};

static NodeT *GetNode(TreeT *tree) {
  return tree ? (&tree->node) : NULL;
}

static NodeT *GetChildren(TreeT *tree) {
  return tree ? (&tree->children) : NULL;
}

static PtrT GetData(NodeT *node) {
  return node ? ((TreeT *)node)->data : NULL;
}

static void DeleteTree(TreeT *tree) {
  NodeUnlink(GetNode(tree));
  MemUnref(tree->data);
}

static void TreeDeleter(TreeT *tree) {
  NodeForEach(GetChildren(tree), (IterFuncT)TreeDeleter, NULL);
  MemUnref(tree);
}

static void RecursiveDeleteTree(TreeT *tree) {
  DeleteTree(tree);
  NodeForEach(GetChildren(tree), (IterFuncT)TreeDeleter, NULL);
}

static NodeT *NodeAlloc(TreeT *parent, PtrT data) {
  TreeT *item = NewRecordGC(TreeT, (FreeFuncT)DeleteTree);
  NodeInitGuard(GetChildren(item));
  item->parent = parent;
  item->data = data;
  return GetNode(item);
}

static PtrT NodeFree(NodeT *node) {
  PtrT data = GetData(node);
  MemUnref(node);
  return data;
}

TreeT *NewTree(PtrT data) {
  TreeT *tree = NewRecordGC(TreeT, (FreeFuncT)RecursiveDeleteTree);
  NodeInitGuard(GetChildren(tree));
  tree->data = data;
  return data;
}

typedef void (*TreeRecursiveFuncT)(TreeT *node, IterFuncT func, PtrT data);

static void TreeForEachRecursive(TreeT *node, IterFuncT func, PtrT data,
                                 TreeRecursiveFuncT recurse) {
  void NodeRecurse(TreeT *child) {
    recurse(child, func, data);
  }

  NodeForEach(GetChildren(node), (IterFuncT)NodeRecurse, NULL);
}

void TreeForEachNode(TreeT *tree, IterFuncT func, PtrT data) {
  func(tree->data, data);
  TreeForEachRecursive(tree, func, data, TreeForEachNode);
}

void TreeForEachChild(TreeT *tree, IterFuncT func, PtrT data) {
  return NodeForEach(GetChildren(tree), func, data);
}

PtrT TreeGetChild(TreeT *parent, ssize_t index) {
  return GetData(NodeGet(GetNode(parent), index));
}

PtrT TreePopChild(TreeT *parent, ssize_t index) {
  return NodeFree(NodeGet(GetNode(parent), index));
}

void TreeInsertAt(TreeT *parent, PtrT data, ssize_t index) {
  NodePrepend(NodeGet(GetNode(parent), index), NodeAlloc(parent, data));
}

bool TreeIsLeaf(TreeT *tree) {
  return IsGuardEmpty(GetChildren(tree));
}
