#ifndef RTREE_RANGESEARCH_HEAD__
#define RTREE_RANGESEARCH_HEAD__

#include "objects/RTree/RTree.h"
#include "objects/RTree/RangeSearchContext.h"

/*** public ***/
extern RangeSearchContext *rangeSearchRTree(RTree *rt, RangeSearchContext *rsc);

/*** private ***/
#ifdef RTREE_RANGESEARCH_SOURCE__
static RangeSearchContext *RangeSearchDataIdRecursive(RTreeNode *node, Mbr *qmbr, RangeSearchContext *rsc);
#endif //RTREE_RANGESEARCH_SOURCE__

#endif //RTREE_RANGESEARCH_HEAD__
