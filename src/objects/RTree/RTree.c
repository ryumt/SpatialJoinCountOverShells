#define RTREE_FAMILY__
#define RTREE_SOURCE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "objects/RTree/RTree.h"
#include "objects/RTree/Build.h"


inline 
RTree *
constructRTree(void)
{
	RTree *rt;
	
	rt = (void *)malloc(sizeof(*rt));
	if (rt == NULL)
		return NULL;
	rt->root = constructRTreeNode(LEAF);
	rt->level = 0;
		
	return rt;
}

inline 
void 
destructRTree(RTree *rt)
{
	destructRTreeNodeRecursive(rt->root);
	free(rt);
}

inline
void 
insertDataRTree(RTree *rt, ulong_t id, float p[])
{
	RTreeEntry entry;
	
	bzero(&entry, sizeof(entry));
	memcpy(entry.mbr.low, p, sizeof(*p) * NDIMS);
	memcpy(entry.mbr.upp, p, sizeof(*p) * NDIMS);
	entry.id = id;
	
	/********** ID1 **********/
	InsertRTree(rt, &entry, 0);
	/**********     **********/
}
