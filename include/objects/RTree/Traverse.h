#ifndef RTREE_TRAVERSE_HEAD__
#define RTREE_TRAVERSE_HEAD__

#include "objects/RTree/RTree.h"

/*** public ***/
extern void traverseRTree(RTree *rst);

/*** private ***/
#ifdef RTREE_TRAVERSE_SOURCE__
static void TraverseRecursive(RTreeNode *node, int level);
#endif

#endif //RTREE_TRAVERSE_HEAD__
