#include "std/memory.h"
#include "std/list.h"
#include "std/tree.h"

struct TreeNode {
  TreeNodeT *parent;
  ListT *children;
  PtrT item;
};

TreeNodeT *NewTreeNode(TreeNodeT *parent, bool isLeaf, PtrT item) {
  TreeNodeT *node = NEW_S(TreeNodeT);

  node->parent = parent;
  node->item = item;

  if (!isLeaf)
    node->children = NewList();

  return node;
}

PtrT DeleteTreeNode(TreeNodeT *node) {
  if (node) {
    PtrT item = node->item;
    DeleteList(node->children);
    DELETE(node);
    return item;
  }

  return NULL;
}

void TreeNodeDetachFromParent(TreeNodeT *node) {
  if (node->parent) {
    bool FindItem(TreeNodeT *this) { return this == node; }

    ListRemove(node->parent->children, (SearchFuncT)FindItem, NULL);
  }
}

static void RecursiveDeleter(TreeNodeT *node) {
  if (node->children)
    ListForEach(node->children, (IterFuncT)DeleteTreeNode, NULL);
  DeleteList(node->children);
  DeleteTreeNode(node);
}

void DeleteTreeNodeRecursive(TreeNodeT *node) {
  TreeNodeDetachFromParent(node);
  RecursiveDeleter(node);
}

bool TreeNodeIsLeaf(TreeNodeT *node) {
  return BOOL(!node->children);
}

typedef void (*TreeRecursiveFuncT)(TreeNodeT *node, IterFuncT func, PtrT data);

static void TreeForEachRecursive(TreeNodeT *node, IterFuncT func, PtrT data, TreeRecursiveFuncT recurse) {
  void NodeRecurse(TreeNodeT *child) {
    recurse(child, func, data);
  }

  ListForEach(node->children, (IterFuncT)NodeRecurse, NULL);
}

void TreeForEachTopDown(TreeNodeT *node, IterFuncT func, PtrT data) {
  func(node->item, data);
  TreeForEachRecursive(node, func, data, TreeForEachTopDown);
}

void TreeForEachBottomUp(TreeNodeT *node, IterFuncT func, PtrT data) {
  TreeForEachRecursive(node, func, data, TreeForEachBottomUp);
  func(node->item, data);
}

void TreeForEachToRoot(TreeNodeT *node, IterFuncT func, PtrT data) {
  while (node) {
    func(node->item, data);
    node = node->parent;
  }
}
