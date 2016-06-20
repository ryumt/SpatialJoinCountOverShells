#define RTREE_FAMILY__
#define RTREE_MULTIRANGECOUNT_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "objects/RTree/MultiRangeCount.h"
#include "objects/RTree/Distance.h"


inline 
void 
multiRangeCountWithPriodicBoundRTree(RTree *rt, MultiRangeCountContext *mrcc)
{
	Mbr qmbr = {{}, {}};
	float *central = mrcc->central;
	float radius = sqrt(mrcc->radiuses[mrcc->nrads - 1]);
	
	/* out of bound flag for each dimension */
	uint32_t min_oob[4] = { 0, 0, 0, 0 };
	uint32_t max_oob[4] = { 0, 0, 0, 0 };
	float moved_central[4] = {};
	int i, j, k;
	
	/* no periodic bound search */
	setQueryMbr(&qmbr, mrcc->central, radius);
	MultiRangeCountRecursive(rt->root, &qmbr, mrcc);
	
	/* check if query MBR is out of bound on each dimension */
	checkMinMaxOutOfBoundMbr(&qmbr, min_oob, max_oob);
	
	/* prepare for search under periodic bound condition */
	memcpy(moved_central, mrcc->central, sizeof(*moved_central) * NDIMS);
	mrcc->central = moved_central;
	/* to do periodic bound search: move query MBR, then search */
	for (i = 0; i < NDIMS; i++) {
		if (min_oob[i])
			moved_central[i] += UPP_BOUND;
		else if (max_oob[i])
			moved_central[i] -= UPP_BOUND;
		else
			continue;
		setQueryMbr(&qmbr, moved_central, radius);
		MultiRangeCountRecursive(rt->root, &qmbr, mrcc);
		
		for (j = i + 1; j < NDIMS; j++) {
			if (min_oob[j])
				moved_central[j] += UPP_BOUND;
			else if (max_oob[j])
				moved_central[j] -= UPP_BOUND;
			else
				continue;
			setQueryMbr(&qmbr, moved_central, radius);
			MultiRangeCountRecursive(rt->root, &qmbr, mrcc);
			
			for (k = j + 1; k < NDIMS; k++) {
				if (min_oob[k])
					moved_central[k] += UPP_BOUND;
				else if (max_oob[k])
					moved_central[k] -= UPP_BOUND;
				else
					continue;
				setQueryMbr(&qmbr, moved_central, radius);
				MultiRangeCountRecursive(rt->root, &qmbr, mrcc);
				moved_central[k] = central[k];
			}
			moved_central[j] = central[j];
		}
		moved_central[i] = central[i];
	}
	// mrcc->central = central;
}

inline 
void 
multiRangeCountRTree(RTree *rt, MultiRangeCountContext *mrcc)
{
	Mbr qmbr = {{}, {}};
	
	setQueryMbr(&qmbr, mrcc->central, sqrt(mrcc->radiuses[mrcc->nrads - 1]));
	MultiRangeCountRecursive(rt->root, &qmbr, mrcc);
}

static inline 
void 
MultiRangeCountRecursive(RTreeNode *node, Mbr *qmbr, MultiRangeCountContext *mrcc)
{
	int k;
	
	if (node->type != LEAF) {
		for (k = 0; k < node->nchilds; k++) {
			if (checkOverlapMbr(&node->mbrs[k], qmbr) == true)
				MultiRangeCountRecursive(node->childs[k], qmbr, mrcc);
		}
	}
	else { 
		for (k = 0; k < node->nchilds; k++) {
			float dist;
			int i;
			
			dist = CalcDistance(node->mbrs[k].low, mrcc->central);
			if (dist == 0)
				return ;
			for (i = 0; i < mrcc->nrads; i++) {
				if (dist < mrcc->radiuses[i]) {
					mrcc->counts[i]++;
					break;
				}
			}
		}
	}
}

