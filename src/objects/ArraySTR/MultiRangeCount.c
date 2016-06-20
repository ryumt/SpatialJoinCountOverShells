#define ARRAYSTR_FAMILY__
#define ARRAYSTR_MULTIRANGECOUNT_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "objects/ArraySTR/MultiRangeCount.h"
#include "objects/RTree/Distance.h"


inline 
void 
multiRangeCountWithPriodicBoundArraySTR(ArraySTR *art, MultiRangeCountContext *mrcc)
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
	MultiRangeCountRecursive(art, art->root, &qmbr, mrcc);
	
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
		MultiRangeCountRecursive(art, art->root, &qmbr, mrcc);
		
		for (j = i + 1; j < NDIMS; j++) {
			if (min_oob[j])
				moved_central[j] += UPP_BOUND;
			else if (max_oob[j])
				moved_central[j] -= UPP_BOUND;
			else 
				continue;
			setQueryMbr(&qmbr, moved_central, radius);
			MultiRangeCountRecursive(art, art->root, &qmbr, mrcc);
			
			for (k = j + 1; k < NDIMS; k++) {
				if (min_oob[k])
					moved_central[k] += UPP_BOUND;
				else if (max_oob[k])
					moved_central[k] -= UPP_BOUND;
				else 
					continue;
				setQueryMbr(&qmbr, moved_central, radius);
				MultiRangeCountRecursive(art, art->root, &qmbr, mrcc);
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
multiRangeCountArraySTR(ArraySTR *art, MultiRangeCountContext *mrcc)
{
	Mbr qmbr = {{}, {}};
	
	setQueryMbr(&qmbr, mrcc->central, mrcc->radiuses[mrcc->nrads - 1]);
	MultiRangeCountRecursive(art, art->root, &qmbr, mrcc);
}

static inline 
void 
MultiRangeCountRecursive(ArraySTR *art, ulong_t inode, Mbr *qmbr, MultiRangeCountContext *mrcc)
{
	ArraySTRNode *node = &art->nodes[inode];
	
	/* non-leaf node */
	if (inode < art->ni4leaf.fidx) {
		ArraySTRNode *child = &art->nodes[node->pos];
		ulong_t k;
		
		//if (checkOverlapMbr(&child->mbr, qmbr) == true)
		//MultiRangeCountRecursive(art, node->pos, qmbr, mrcc);
		for (k = 0; k < node->len; k++) {
			if (checkOverlapMbr(&child[k].mbr, qmbr) == true)
				MultiRangeCountRecursive(art, node->pos + k, qmbr, mrcc);
		}
	}
	else { 
		float dist;
		int i;
		
		dist = CalcDistance(node->mbr.low, mrcc->central);
		for (i = 0; i < mrcc->nrads; i++) {
			if (dist < mrcc->radiuses[i]) {
				mrcc->counts[i]++;
				break;
			}
		}
	}
}
