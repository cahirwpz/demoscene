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

  if (isLeaf)
    node->children = NewList();

  return node;
}

static bool DeleteTreeNodeRecursive(TreeNodeT *node) {
  if (node->children) {
    ListForEach(node->children, (IterFuncT)DeleteTreeNodeRecursive, NULL);
    DeleteList(node->children);
  }
  DELETE(node);
  return TRUE;
}

void DeleteTreeNode(TreeNodeT *node) {
  if (node->parent) {
    /* TODO */
  }

  DeleteTreeNodeRecursive(node);
}

bool TreeNodeIsLeaf(TreeNodeT *node) {
  return BOOL(!node->children);
}

typedef void* (*TreeRecursiveFuncT)(TreeNodeT *node, IterFuncT func, void *data);

static void *TreeForEachRecursive(TreeNodeT *node, IterFuncT func, void *data, TreeRecursiveFuncT recurse) {
  void *item = NULL;

  bool NodeRecurse(TreeNodeT *child) {
    void *stopItem = recurse(child, func, data);

    if (stopItem && !item)
      item = stopItem;

    return BOOL(item);
  }

  ListForEach(node->children, (IterFuncT)NodeRecurse, NULL);

  return item;
}

void *TreeForEachTopDown(TreeNodeT *node, IterFuncT func, void *data) {
  void *item = NULL;

  if (!func(node->item, data)) {
    item = node->item;
  } else {
    item = TreeForEachRecursive(node, func, data, TreeForEachTopDown);
  }

  return item;
}

void *TreeForEachBottomUp(TreeNodeT *node, IterFuncT func, void *data) {
  void *item = TreeForEachRecursive(node, func, data, TreeForEachBottomUp);

  if (!item) {
    if (!func(node->item, data))
      item = node->item;
  }

  return item;
}
