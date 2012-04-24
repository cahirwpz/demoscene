#include "std/memory.h"
#include "std/tree.h"

static NodeT *GetNode(TreeT *tree) {
  return tree ? (&tree->node) : NULL;
}

static NodeT *GetChildren(TreeT *tree) {
  return tree ? (&tree->children) : NULL;
}

static PtrT GetData(NodeT *node) {
  return node ? ((TreeT *)node)->data : NULL;
}

static NodeT *NodeAlloc(TreeT *parent, PtrT data) {
  TreeT *item = MemNew0(sizeof(TreeT));
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
  TreeT *tree = MemNew0(sizeof(TreeT));
  NodeInitGuard(GetChildren(tree));
  tree->data = data;
  return data;
}

static void TreeDeleter(TreeT *tree, FreeFuncT func) {
  NodeForEach(GetChildren(tree), (IterFuncT)TreeDeleter, (PtrT)func);
  func(tree);
}

void DeleteTree(TreeT *tree) {
  if (tree)
    DeleteTreeFull(tree, (FreeFuncT)MemUnref);
}

void DeleteTreeFull(TreeT *tree, FreeFuncT func) {
  TreeDeleter(tree, func);
  func(NodeUnlink(GetNode(tree)));
}

typedef void (*TreeRecursiveFuncT)(TreeT *node, IterFuncT func, PtrT data);

static void TreeForEachRecursive(TreeT *node, IterFuncT func, PtrT data, TreeRecursiveFuncT recurse) {
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
