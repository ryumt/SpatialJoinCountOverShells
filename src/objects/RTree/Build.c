#define RTREE_FAMILY__
#define RTREE_BUILD_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "objects/RTree/Build.h"


typedef struct NodeCostPair {
	RTreeNode *node;
	float cost;
} NodeCostPair;

static 
int 
ComparNodeCostPair(const void *a, const void *b)
{
	const NodeCostPair *ncp1 = a;
	const NodeCostPair *ncp2 = b;
	
	if (ncp1->cost < ncp2->cost)
		return -1;
	else if (ncp1->cost > ncp2->cost)
		return 1;
	else 
		return 0;
}

static inline
void 
RemoveEntry(RTreeEntry entries[], int rm_index, int remain)
{
	RTreeEntry *target = &entries[rm_index];
	size_t length = sizeof(*entries) * (remain - rm_index - 1);
	memmove(target, target + 1, length);
}


static inline 
void 
PickSeeds(RTreeEntry entries[], RTreeNode *seed1, RTreeNode *seed2)
{
	float max_diff = -FLT_MAX;
	int max_pair[2] = {0, 1};
	int i, j;
	
	/********** PS1 **********/
	for (i = 0; i < MAX_ENT; i++) {
		for (j = i + 1; j < MAX_ENT + 1; j++) {
			Mbr composed = entries[i].mbr;
			float diff;
			
			enlargeMbr(&composed, &entries[j].mbr);
			diff = calculateAreaMbr(&composed)
				- calculateAreaMbr(&entries[i].mbr)
				- calculateAreaMbr(&entries[j].mbr);
			/********** PS2 **********/
			if (diff > max_diff) {
				max_diff = diff;
				max_pair[0] = i;
				max_pair[1] = j;
			}
			/**********     **********/
		}
	}
	/**********     **********/
	addChildEntryToRTreeNode(seed1, &entries[max_pair[0]]);
	addChildEntryToRTreeNode(seed2, &entries[max_pair[1]]);
	RemoveEntry(entries, max_pair[0], MAX_ENT + 1);
	RemoveEntry(entries, max_pair[1] - 1, (MAX_ENT + 1) - 1);
}

static inline 
int 
DistributeEntry(RTreeEntry entries[], int remain, RTreeNode *node1, RTreeNode *node2)
{
	RTreeNode *target;
	int idx;
	
	/********** DE1 **********/
	idx = PickNext(entries, remain, node1, node2);
	/**********     **********/
	
	/********** DE2 **********/
	if (idx >= MAX_ENT) {
		idx = (idx - 1) - MAX_ENT;
		target = (node1->nchilds < node2->nchilds) ? node1 : node2;
	}
	else if (idx < 0) {
		idx = -idx - 1;
		target = node1;
	}
	else { /* if (idx > 0) { */
		idx = idx - 1;
		target = node2;
	}
	addChildEntryToRTreeNode(target, &entries[idx]);
	/**********     **********/
	return idx;
}

static inline 
int 
PickNext(RTreeEntry entries[], int remain, RTreeNode *node1, RTreeNode *node2)
{
	float max_diff = -FLT_MAX;
	int max_idx = 0;
	Mbr mbr1, mbr2;
	int i;
	
	mbr1 = getMbrRTreeNode(node1);
	mbr2 = getMbrRTreeNode(node2);
	/********** PN1 **********/
	for (i = 0; i < remain; i++) {
		Mbr composed;
		float d1, d2, diff;
		
		composed = mbr1;
		enlargeMbr(&composed, &entries[i].mbr);
		d1 = calculateAreaMbr(&composed) - calculateAreaMbr(&mbr1);
		composed = mbr2;
		enlargeMbr(&composed, &entries[i].mbr);
		d2 = calculateAreaMbr(&composed) - calculateAreaMbr(&mbr2);
		
		diff = fabs(d1 - d2);
		/********** PN2 **********/
		if (diff > max_diff) {
			max_diff = diff;
			/* compare enlargement */
			if (d1 < d2)
				max_idx = -(i + 1);
			else if (d2 < d1)
				max_idx = (i + 1);
			else 
				max_idx = MAX_ENT + (i + 1);
		}
		/**********     **********/
	}
	/**********     **********/	
	return max_idx;
}


static inline 
void 
SplitNode(RTreeNode *node, RTreeNode **splitp, RTreeEntry *entry)
{
	RTreeEntry entries[MAX_ENT + 1];
	int remain = MAX_ENT + 1;
	RTreeNode *remainder;
	int i;
	
	for (i = 0; i < MAX_ENT; i++)
		initRTreeEntry(&entries[i], &node->mbrs[i], node->childs[i]);
	entries[MAX_ENT] = *entry;
	
	/********** QS1 **********/
	node->nchilds = 0;
	*splitp = constructRTreeNode(node->type);
	PickSeeds(entries, node, *splitp);
	remain -= 2;
	/**********    **********/
	
	/********** QS2 **********/
	remainder = NULL;
	for ( ; remain > 0; remain--) {
		int idx;
		
		if (node->nchilds == MAX_ENT - MIN_ENT + 1) {
			remainder = *splitp;
			break;
		}
		if ((*splitp)->nchilds == MAX_ENT - MIN_ENT + 1) {
			remainder = node;
			break;
		}
		idx = DistributeEntry(entries, remain, node, *splitp);
		RemoveEntry(entries, idx, remain);
	}
	/**********    **********/
	
	/********** QS3 **********/
	for (i = 0; i < remain; i++)
		addChildEntryToRTreeNode(remainder, &entries[i]);
	/**********    **********/
}

static inline 
void 
AdjustTree(RTreeNode *node, RTreeNode **splitp, RTreeEntry *entry)
{
	RTreeNode *parent;
	
	/********** AT2 **********/
	for ( ; node->parent != NULL; node = parent) {
		int idx;
		
		/********** AT3 **********/
		parent = node->parent;
		idx = findEntryIndexFromParentRTreeNode(parent, node);
		parent->mbrs[idx] = getMbrRTreeNode(node);
		/**********     **********/
		
		/********** AT4 **********/
		if (*splitp != NULL) {
			RTreeEntry split_entry = {
				getMbrRTreeNode(*splitp), { *splitp }
			};
			
			if (parent->nchilds < MAX_ENT) {
				addChildEntryToRTreeNode(parent, &split_entry);
				*splitp = NULL;
			}
			else
				SplitNode(parent, splitp, &split_entry);
		}
		/**********     **********/
	}
	/**********     **********/
}

static inline 
void 
SplitRoot(RTree *rt, RTreeNode *old, RTreeNode *split)
{
	RTreeNode *root;
	RTreeEntry new_entry;
	
	root = constructRTreeNode(NON_LEAF);
	new_entry.mbr = getMbrRTreeNode(old);
	new_entry.node = old;
	addChildEntryToRTreeNode(root, &new_entry);
	new_entry.mbr = getMbrRTreeNode(split);
	new_entry.node = split;
	addChildEntryToRTreeNode(root, &new_entry);
	rt->root = root;
	rt->level++;
}

inline 
void 
InsertRTree(RTree *rt, RTreeEntry *entry, ulong_t level)
{
	RTreeNode *target;
	RTreeNode *split = NULL; 
	
	/********** I1 **********/
	target = ChooseSubTree(rt, entry, level);
	/**********     **********/
	
	/********** I2 **********/
	if (target->nchilds < MAX_ENT)
		addChildEntryToRTreeNode(target, entry);
	else
		SplitNode(target, &split, entry);
	/**********     **********/
	
	/********** I3 **********/
	AdjustTree(target, &split, entry);
	/**********     **********/
	
	/********** I4 **********/
	if (split != NULL)
		SplitRoot(rt, rt->root, split);
	/**********     **********/
}

static inline 
RTreeNode * 
ChooseSubTree(RTree *rt, RTreeEntry *entry, ulong_t leaf_level)
{
	NodeCostPair ncp[MAX_ENT];
	RTreeNode *node;
	ulong_t level;
	
	/********** CS1 **********/
	node = rt->root;
	/**********     **********/
	
	/********** CS2 **********/
	for (level = rt->level - leaf_level; level > 0; level--) {
		int i, n;
		
		bzero(ncp, sizeof(ncp));
		/* calculate area enlargement cost */
		for (i = 0; i < node->nchilds; i++) {
			Mbr expanded = node->mbrs[i];
			
			enlargeMbr(&expanded, &entry->mbr);
			ncp[i].node = node->childs[i];
			ncp[i].cost = fabs(calculateAreaMbr(&expanded) - calculateAreaMbr(&node->mbrs[i]));
		}
		/* sort area enlargement cost to determin minimum cost */
		qsort(ncp, node->nchilds, sizeof(*ncp), ComparNodeCostPair);
		
		/* examin whether there are more than one minimum cost */
		for (n = 1; n < node->nchilds; n++) {
			if (ncp[n].cost != ncp[n - 1].cost)
				break;
		}
		if (n > 1) {
			/* calculate area cost */
			for (i = 0; i < n; i++) {
				int idx = findEntryIndexFromParentRTreeNode(node, ncp[i].node);
				ncp[i].cost = calculateAreaMbr(&node->mbrs[idx]);
			}
			/* sort area cost to determin minimum cost */
			qsort(ncp, n, sizeof(*ncp), ComparNodeCostPair);
		}
		/********** CS3 **********/
		node = ncp[0].node;
		/**********     **********/
	}
	/**********     **********/
		
	return node;
}

