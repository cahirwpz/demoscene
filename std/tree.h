#ifndef __STD_TREE_H__
#define __STD_TREE_H__

#include "std/types.h"
#include "std/list.h"

/**
 * A tree is useful for expressing hierarchical structures like filesystems and
 * three-dimensional scenes.
 */
typedef struct TreeNode TreeNodeT;

TreeNodeT *NewTreeNode(TreeNodeT *parent, bool isLeaf, PtrT item);
PtrT DeleteTreeNode(TreeNodeT *node);
void DeleteTreeNodeRecursive(TreeNodeT *node);

bool TreeNodeIsLeaf(TreeNodeT *node);

void TreeForEachTopDown(TreeNodeT *node, IterFuncT func, PtrT data);
void TreeForEachBottomUp(TreeNodeT *node, IterFuncT func, PtrT data);
void TreeForEachToRoot(TreeNodeT *node, IterFuncT func, PtrT data);

#endif
