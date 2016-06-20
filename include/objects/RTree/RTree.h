#ifndef RTREE_HEAD__
#define RTREE_HEAD__

#include <stdbool.h>
#include "types/ulong.h"

/*** friend public ***/
#ifdef RTREE_FAMILY__
#include "objects/RTree/Node.h"

typedef struct RTree {
	RTreeNode *root;
	ulong_t level;
} RTree;

/*** public ***/
#else
typedef struct RTree RTree;
#endif //RTREE_FAMILY__

/*** public ***/
extern RTree *constructRTree(void);
extern void destructRTree(RTree *rt);

extern void insertDataRTree(RTree *rt,  ulong_t id, float p[]);

#endif //RTREE_HEAD__
