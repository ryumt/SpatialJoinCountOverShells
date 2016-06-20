#ifndef RTREE_NODE_HEAD__
#define RTREE_NODE_HEAD__

#include "types/ulong.h"
#include "objects/RTree/Mbr.h"

#define MAX_ENT (10)
#define MIN_ENT (4)

typedef enum RTreeNodeType {
	NON_LEAF, LEAF
} RTreeNodeType;

typedef struct RTreeNode {
	struct RTreeNode *parent;
	RTreeNodeType type;
	
	int nchilds;
	Mbr mbrs[MAX_ENT];
	union {
		struct RTreeNode *childs[MAX_ENT];
		ulong_t ids[MAX_ENT];
	};
} RTreeNode;

typedef struct RTreeEntry {
	Mbr mbr;
	union {
		RTreeNode *node;
		ulong_t id;
	};
} RTreeEntry;

extern RTreeNode *constructRTreeNode(RTreeNodeType type);
extern void destructRTreeNode(RTreeNode *node);
extern void destructRTreeNodeRecursive(RTreeNode *node);
extern int findEntryIndexFromParentRTreeNode(RTreeNode *parent, RTreeNode *node);
extern int addChildEntryToRTreeNode(RTreeNode *node, RTreeEntry *entry);
extern Mbr getMbrRTreeNode(RTreeNode *node);
extern void initRTreeEntry(RTreeEntry *entry, Mbr *mbrp, RTreeNode *node);
#endif //RTREE_NODE_HEAD__
