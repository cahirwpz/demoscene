#ifndef __STD_TREE_H__
#define __STD_TREE_H__

#include "std/list.h"

/**
 * A tree is useful for expressing hierarchical structures like filesystems and
 * three-dimensional scenes.
 */
typedef struct TreeNode {
  struct TreeNode *parent;
  ListT *children;
  void *item;
} TreeNodeT;

TreeNodeT *NewTreeNode(TreeNodeT *parent, bool isLeaf, void *item);
void DeleteTreeNode(TreeNodeT *node);

void *TreeForEachTopDown(TreeNodeT *node, IterFuncT func, void *data);
void *TreeForEachBottomUp(TreeNodeT *node, IterFuncT func, void *data);

static inline bool TreeNodeIsLeaf(TreeNodeT *node) { return BOOL(!node->children); }

#endif
