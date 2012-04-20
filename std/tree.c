#include "std/memory.h"
#include "std/list.h"
#include "std/tree.h"

struct TreeNode {
  TreeNodeT *parent;
  ListT *children;
  void *item;
};

TreeNodeT *NewTreeNode(TreeNodeT *parent, bool isLeaf, void *item) {
  TreeNodeT *node = NEW_S(TreeNodeT);

  node->parent = parent;
  node->item = item;

  if (!isLeaf)
    node->children = NewList();

  return node;
}

void* DeleteTreeNode(TreeNodeT *node) {
  if (node) {
    void *item = node->item;
    DeleteList(node->children);
    DELETE(node);
    return item;
  }

  return NULL;
}

static void TreeNodeRecursiveDelete(TreeNodeT *node) {
  if (node->children)
    ListForEach(node->children, (IterFuncT)TreeNodeRecursiveDelete, NULL);
  DeleteList(node->children);
  DeleteTreeNode(node);
}

void DeleteTreeNodeRecursive(TreeNodeT *node) {
  if (node->parent) {
    bool FindItem(TreeNodeT *this) { return this == node; }

    ListRemove(node->parent->children, (SearchFuncT)FindItem, NULL);
  }
  TreeNodeRecursiveDelete(node);
  DeleteTreeNode(node);
}

bool TreeNodeIsLeaf(TreeNodeT *node) {
  return BOOL(!node->children);
}

static void *TreeNodeGetItem(TreeNodeT *node) {
  return node->item;
}

TreeNodeT *TreeNodeGetParent(TreeNodeT *node) {
  return node->parent;
}

ListT *TreeNodeGetChildren(TreeNodeT *node) {
  return node->children;
}

typedef void (*TreeRecursiveFuncT)(TreeNodeT *node, IterFuncT func, void *data);

static void TreeForEachRecursive(TreeNodeT *node, IterFuncT func, void *data, TreeRecursiveFuncT recurse) {
  void NodeRecurse(TreeNodeT *child) {
    recurse(child, func, data);
  }

  ListForEach(TreeNodeGetChildren(node), (IterFuncT)NodeRecurse, NULL);
}

void TreeForEachTopDown(TreeNodeT *node, IterFuncT func, void *data) {
  func(TreeNodeGetItem(node), data);
  TreeForEachRecursive(node, func, data, TreeForEachTopDown);
}

void TreeForEachBottomUp(TreeNodeT *node, IterFuncT func, void *data) {
  TreeForEachRecursive(node, func, data, TreeForEachBottomUp);
  func(TreeNodeGetItem(node), data);
}

void TreeForEachToRoot(TreeNodeT *node, IterFuncT func, void *data) {
  while (node) {
    func(TreeNodeGetItem(node), data);
    node = TreeNodeGetParent(node);
  }
}
