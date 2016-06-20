#define RTREE_FAMILY__
#define RTREE_TRAVERSE_SOURCE__

#include <stdio.h>

#include "objects/RTree/Traverse.h"

static ulong_t VisitedId;


inline 
void 
traverseRTree(RTree *rt)
{
	Mbr mbr = getMbrRTreeNode(rt->root);
	VisitedId = 0;
	printf("[%5lu]: type=%d, {%f %f %f} ~ {%f %f %f}\n", 
	       VisitedId++, rt->root->type, 
	       mbr.low[0], mbr.low[1], mbr.low[2], 
	       mbr.upp[0], mbr.upp[1], mbr.upp[2]);
	TraverseRecursive(rt->root, 0);
}

static inline
void 
TraverseRecursive(RTreeNode *node, int level)
{
	int i, k;
	
	for (k = 0; k < node->nchilds; k++) {
		for (i = 0; i < level; i++)
			printf("  ");
		printf("[%5lu]: type=%d, {%f %f %f} ~ {%f %f %f}\n", 
		       VisitedId++, (node->type != LEAF) ? node->childs[k]->type : -1, 
		       node->mbrs[k].low[0], node->mbrs[k].low[1], node->mbrs[k].low[2], 
		       node->mbrs[k].upp[0], node->mbrs[k].upp[1], node->mbrs[k].upp[2]);
	}
	
	if (node->type != LEAF) {
		for (k = 0; k < node->nchilds; k++)
			TraverseRecursive(node->childs[k], level + 1);
	}
}
