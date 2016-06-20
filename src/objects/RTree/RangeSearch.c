#define RTREE_FAMILY__
#define RTREE_RANGESEARCH_SOURCE__

#include <stdio.h>
#include <stdlib.h>

#include "objects/RTree/RangeSearch.h"
#include "objects/RTree/Distance.h"


inline 
RangeSearchContext *
rangeSearchRTree(RTree *rst, RangeSearchContext *rsc)
{
	Mbr qmbr;
			
	rsc->nids = 0;
	if (rsc->radius < 0)
		return rsc;
	
	setQueryMbr(&qmbr, rsc->central, rsc->radius);
	return RangeSearchDataIdRecursive(rst->root, &qmbr, rsc);
}

static inline 
RangeSearchContext *
RangeSearchDataIdRecursive(RTreeNode *node, Mbr *qmbr, RangeSearchContext *rsc)
{
	int k;
	
	if (node->type != LEAF) {
		for (k = 0; k < node->nchilds; k++) {
			if (checkOverlapMbr(&node->mbrs[k], qmbr) == true)
                                rsc = RangeSearchDataIdRecursive(node->childs[k], qmbr, rsc);
		}
	}
	else { 
		for (k = 0; k < node->nchilds; k++) {
			if (RangeSearchValidation(node->mbrs[k].low, rsc->central, rsc->radius) == true) {
				if ((rsc = reallocRangeSearchContext(rsc, rsc->nids + 1)) == NULL)
					return NULL;
				rsc->ids[rsc->nids++] = node->ids[k];
			}
		}
	}
	
	return rsc;
}
