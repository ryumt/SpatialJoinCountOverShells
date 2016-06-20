#define RTREE_NODE_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <x86intrin.h>

#include "objects/RTree/Node.h"


inline 
RTreeNode *
constructRTreeNode(RTreeNodeType type)
{
	RTreeNode *node;
	
	node = (void *)calloc(1, sizeof(*node));
	if (node == NULL)
		return NULL;
	node->type = type;
	
	return node;
}

inline 
void 
destructRTreeNode(RTreeNode *node)
{
	free(node);
}

inline
void
destructRTreeNodeRecursive(RTreeNode *node)
{
        int i;
	
	if (node->type != LEAF) {
		for (i = 0; i < node->nchilds; i++)
                        destructRTreeNodeRecursive(node->childs[i]);
        }
        destructRTreeNode(node);
}

inline
int 
findEntryIndexFromParentRTreeNode(RTreeNode *parent, RTreeNode *node)
{
	int i;
	
	for (i = 0; i < parent->nchilds; i++) {
		if (parent->childs[i] == node)
			return i;
	}
	return i;
}

inline 
int 
addChildEntryToRTreeNode(RTreeNode *node, RTreeEntry *entry)
{
	if (node->nchilds >= MAX_ENT)
		return -1;
	
	node->mbrs[node->nchilds] = entry->mbr;
	if (node->parent != NULL) {
		int idx = findEntryIndexFromParentRTreeNode(node->parent, node);
		node->parent->mbrs[idx] = getMbrRTreeNode(node);
	}
		
	
	node->childs[node->nchilds] = entry->node;
	node->nchilds++;
	if (node->type != LEAF)
		entry->node->parent = node;
	return 0;
}

inline 
Mbr 
getMbrRTreeNode(RTreeNode *node)
{
	Mbr mbr;
	int k;
	
	mbr = node->mbrs[0];
	for (k = 1; k < node->nchilds; k++) {
#ifdef ENABLE_SSE_TEST1
                __m128 v_nlow = _mm_load_ps(mbr.low);
                __m128 v_nupp = _mm_load_ps(mbr.upp);
                __m128 v_clow = _mm_load_ps(node->mbrs[k].low);
                __m128 v_cupp = _mm_load_ps(node->mbrs[k].upp);
                _mm_store_ps(node->mbr.low, _mm_min_ps(v_nlow, v_clow));
                _mm_store_ps(node->mbr.upp, _mm_max_ps(v_nupp, v_cupp));
#else
#ifdef ENABLE_AVX_TEST1
                __m256 v_nmbr = _mm256_loadu_ps((float *)&mbr);
                __m256 v_cmbr = _mm256_loadu_ps((float *)&node->mbrs[k]);
                __m256 v_min = _mm256_min_ps(v_nmbr, v_cmbr);
                __m256 v_max = _mm256_max_ps(v_nmbr, v_cmbr);
                __m256 v_tmp;
                v_tmp = _mm256_permute2f128_ps(v_min, v_max, 0x12);
                _mm256_storeu_ps((float *)&mbr, v_tmp);
#else
		int i;
                for (i = 0; i < NDIMS; i++) {
                        if (mbr.low[i] > node->mbrs[k].low[i])
				mbr.low[i] = node->mbrs[k].low[i];
                        if (mbr.upp[i] < node->mbrs[k].upp[i])
                                mbr.upp[i] = node->mbrs[k].upp[i];
                }
#endif
#endif
	}
		return mbr;
}

inline 
void 
initRTreeEntry(RTreeEntry *entry, Mbr *mbrp, RTreeNode *node)
{
	entry->mbr = *mbrp;
	entry->node = node;
}

