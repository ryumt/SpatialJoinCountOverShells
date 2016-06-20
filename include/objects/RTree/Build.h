#ifndef RTREE_BUILD_HEAD__
#define RTREE_BUILD_HEAD__

#include "types/ulong.h"
#include "objects/RTree/RTree.h"

/*** private ***/
#ifdef RTREE_BUILD_SOURCE__
static int ComparNodeCostPair(const void *a, const void *b);
static void RemoveEntry(RTreeEntry entries[], int rm_index, int remain);

static void PickSeeds(RTreeEntry entries[], RTreeNode *seed1, RTreeNode *seed2);
static int DistributeEntry(RTreeEntry entries[], int remain, RTreeNode *node1, RTreeNode *node2);
static int PickNext(RTreeEntry entries[], int remain, RTreeNode *node1, RTreeNode *node2);

static RTreeNode *ChooseSubTree(RTree *rt, RTreeEntry *entry, ulong_t sub_level);
static void SplitNode(RTreeNode *node, RTreeNode **splitp, RTreeEntry *entry);
static void SplitRoot(RTree *rt, RTreeNode *old, RTreeNode *split);
static void AdjustTree(RTreeNode *node, RTreeNode **splitp, RTreeEntry *entry);
#endif //RTREE_BUILD_SOURCE__

/*** friend public ***/
#ifdef RTREE_FAMILY__
extern void InsertRTree(RTree *rt, RTreeEntry *entry, ulong_t level);
#endif //RTREE_FAMILY__

#endif//RTREE_BUILD_HEAD__
