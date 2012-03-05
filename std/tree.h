#ifndef __STD_TREE_H__
#define __STD_TREE_H__

#include "std/types.h"

/**
 * A tree is useful for expressing hierarchical structures like filesystems and
 * three-dimensional scenes.
 */
typedef struct TreeNode TreeNodeT;

TreeNodeT *NewTreeNode(TreeNodeT *parent, bool isLeaf, void *item);
void DeleteTreeNode(TreeNodeT *node);

bool TreeNodeIsLeaf(TreeNodeT *node);
TreeNodeT *TreeNodeGetParent(TreeNodeT *node);
ListT *TreeNodeGetChildren(TreeNodeT *node);

void *TreeForEachTopDown(TreeNodeT *node, IterFuncT func, void *data);
void *TreeForEachBottomUp(TreeNodeT *node, IterFuncT func, void *data);
void *TreeForEachToRoot(TreeNodeT *node, IterFuncT func, void *data);

#endif
