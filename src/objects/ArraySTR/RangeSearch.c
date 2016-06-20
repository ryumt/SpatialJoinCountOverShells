#define ARRAYSTR_FAMILY__
#define ARRAYSTR_RANGESEARCH_SOURCE__

#include <stdio.h>
#include <stdlib.h>

#include "objects/ArraySTR/RangeSearch.h"
#include "objects/RTree/Distance.h"


inline 
RangeSearchContext *
rangeSearchArraySTR(ArraySTR *art, RangeSearchContext *rsc)
{
	Mbr qmbr;
			
	rsc->nids = 0;
	if (rsc->radius < 0)
		return rsc;
	
	setQueryMbr(&qmbr, rsc->central, rsc->radius);
	return RangeSearchDataIdRecursive(art, art->root, &qmbr, rsc);
}


static inline 
RangeSearchContext *
RangeSearchDataIdRecursive(ArraySTR *art, ulong_t inode, Mbr *qmbr, RangeSearchContext *rsc)
{
	ArraySTRNode *node = &art->nodes[inode];
	
	/* non-leaf node */
	if (inode < art->ni4leaf.fidx) {
		ArraySTRNode *child = &art->nodes[node->pos];
                ulong_t k;
		
                for (k = 0; k < node->len; k++) {
                        if (checkOverlapMbr(&child->mbr, qmbr) == true)
                                rsc = RangeSearchDataIdRecursive(art, node->pos + k, qmbr, rsc);
			child++;
		}
	}
	else { 
		if (RangeSearchValidation(node->mbr.low, rsc->central, rsc->radius) == true) {
			if ((rsc = reallocRangeSearchContext(rsc, rsc->nids + 1)) == NULL)
				return NULL;
			rsc->ids[rsc->nids++] = node->id;
		}
	}
	
	return rsc;
}
