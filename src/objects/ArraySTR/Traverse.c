#define ARRAYSTR_FAMILY__
#define ARRAYSTR_TRAVERSE_SOURCE__

#include <stdio.h>

#include "objects/ArraySTR/Traverse.h"


inline 
void 
traverseArraySTR(ArraySTR *art)
{
	TraverseRecursive(art, art->root, 0);
}

static inline
void 
TraverseRecursive(ArraySTR *art, ulong_t inode, int level)
{
	ArraySTRNode *node = &art->nodes[inode];
	int i;
	
	for (i = 0; i < level; i++)
		printf("  ");
	printf("[%5lu]: {%f %f %f} ~ {%f %f %f}\n", inode, 
	       node->mbr.low[0], node->mbr.low[1], node->mbr.low[2], 
	       node->mbr.upp[0], node->mbr.upp[1], node->mbr.upp[2]);
	
	if (inode < art->ni4leaf.fidx) {
		ulong_t k;
		for (k = 0; k < node->len; k++)
			TraverseRecursive(art, node->pos + k, level + 1);
	}
}
