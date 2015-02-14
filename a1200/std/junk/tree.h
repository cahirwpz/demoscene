#ifndef __STD_TREE_H__
#define __STD_TREE_H__

#include "std/types.h"
#include "std/node.h"

/**
 * A tree is useful for expressing hierarchical structures like filesystems and
 * three-dimensional scenes.
 */
typedef struct _Tree TreeT;

TreeT *NewTree(PtrT data);

void TreeForEachNode(TreeT *tree, IterFuncT func, PtrT data);
void TreeForEachChild(TreeT *tree, IterFuncT func, PtrT data);

PtrT TreeGetChild(TreeT *parent, ssize_t index);
PtrT TreePopChild(TreeT *parent, ssize_t index);
void TreeInsertAt(TreeT *parent, PtrT data, ssize_t index);

bool TreeIsLeaf(TreeT *parent);

#endif
