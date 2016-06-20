#ifndef RTREE_MULTIRANGECOUNT_HEAD__
#define RTREE_MULTIRANGECOUNT_HEAD__

#include "objects/RTree/RTree.h"
#include "objects/RTree/MultiRangeCountContext.h"

/*** public ***/
extern void multiRangeCountRTree(RTree *rt, MultiRangeCountContext *mrcc);
extern void multiRangeCountWithPriodicBoundRTree(RTree *rt, MultiRangeCountContext *mrcc);

/*** private ***/
#ifdef RTREE_MULTIRANGECOUNT_SOURCE__
static void MultiRangeCountRecursive(RTreeNode *node, Mbr *qmbr, MultiRangeCountContext *mrcc);
#endif

#endif //RTREE_MULTIRANGECOUNT_HEAD__
